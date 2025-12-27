import uvicorn
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
import wave
from utils import transcribe_audio
from dotenv import load_dotenv
from pydantic import BaseModel
import os
from config import SYSTEM_PROMPT, OPENROUTER_URL

load_dotenv()

app = FastAPI()

# Audio Configuration matches ESP32 settings
SAMPLE_RATE = 16000
CHANNELS = 1
SAMPLE_WIDTH = 2  # 2 bytes = 16-bit resolution
audio_data = None

OPENROUTER_API_KEY = os.getenv("OPENROUTER_API_KEY")

HEADERS = {
    "Authorization": f"Bearer {OPENROUTER_API_KEY}",
    "Content-Type": "application/json",
    "HTTP-Referer": "http://localhost",
    "X-Title": "ESP32 Voice Assistant"
}

@app.websocket("/ws/audio-stream")
async def audio_stream_endpoint(websocket: WebSocket):
    await websocket.accept()
    print("ESP32 Connected via WebSocket")
    
    # Create a unique filename based on timestamp
    filename = f"user_recording.wav"
    try:
        # Open WAV file for writing
        # 'wb' mode is write binary
        with wave.open(filename, 'wb') as wav_file:
            # Configure WAV headers
            wav_file.setnchannels(CHANNELS)
            wav_file.setsampwidth(SAMPLE_WIDTH)
            wav_file.setframerate(SAMPLE_RATE)
            
            print(f"Recording to {filename}...")
            
            while True:
                # Receive raw binary PCM data from ESP32
                audio_data = await websocket.receive_bytes()
                # Write received data to WAV file
                wav_file.writeframes(audio_data)
            
    except WebSocketDisconnect:
        print("ESP32 Disconnected - Recording Saved")
        transcribed_text = transcribe_audio(filename)
        print(f"Transcribed Text: {transcribed_text}")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    # Run with: python server.py
    # Listen on 0.0.0.0 to allow access from external devices (ESP32)
    uvicorn.run(app, host="0.0.0.0", port=8000)