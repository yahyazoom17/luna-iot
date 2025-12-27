import uvicorn
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from contextlib import asynccontextmanager
import wave
import torch
import whisper
import asyncio
from concurrent.futures import ThreadPoolExecutor

# --- CONFIGURATION ---
SAMPLE_RATE = 16000
CHANNELS = 1
SAMPLE_WIDTH = 2
MODEL_TYPE = "turbo.en"  # Use "tiny.en" for fastest speed, "base.en" for balance

# --- GLOBAL MODEL LOADER ---
# This ensures the model is loaded ONLY ONCE when the app starts
ml_models = {}

@asynccontextmanager
async def lifespan(app: FastAPI):
    print(f"Loading {MODEL_TYPE} model... Please wait.")
    # Check for GPU
    device = "cuda" if torch.cuda.is_available() else "cpu"
    print(f"Running on: {device}")
    
    # Load model into global dictionary
    ml_models["whisper"] = whisper.load_model(MODEL_TYPE, device=device)
    print("Model loaded! Server is ready.")
    yield
    # Clean up resources if needed
    ml_models.clear()

app = FastAPI(lifespan=lifespan)
executor = ThreadPoolExecutor()

# --- HELPER: RUN BLOCKING AI TASK IN BACKGROUND ---
def run_transcription(filename):
    model = ml_models["whisper"]
    # fp16=False speeds up CPU inference slightly and prevents warnings
    result = model.transcribe(filename, fp16=False) 
    return result["text"]

@app.websocket("/ws/audio-stream")
async def audio_stream_endpoint(websocket: WebSocket):
    await websocket.accept()
    print("ESP32 Connected")
    
    filename = "user_recording.wav"
    
    try:
        with wave.open(filename, 'wb') as wav_file:
            wav_file.setnchannels(CHANNELS)
            wav_file.setsampwidth(SAMPLE_WIDTH)
            wav_file.setframerate(SAMPLE_RATE)
            
            while True:
                data = await websocket.receive_bytes()
                wav_file.writeframes(data)
                
    except WebSocketDisconnect:
        print("Processing audio...")
        start_time = asyncio.get_event_loop().time()
        
        # Run transcription in a separate thread so it doesn't freeze the server
        loop = asyncio.get_event_loop()
        try:
            # We await the result, but the heavy lifting happens in the executor
            text = await loop.run_in_executor(None, run_transcription, filename)
            
            end_time = asyncio.get_event_loop().time()
            duration = end_time - start_time
            
            print(f"\n--- RESULT ({duration:.2f}s) ---")
            print(text.strip())
            print("---------------------------")
            
        except Exception as e:
            print(f"Transcription Error: {e}")

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)