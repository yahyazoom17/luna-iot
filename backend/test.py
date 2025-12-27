import uvicorn
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
import wave
import time

app = FastAPI()

# Audio Configuration matches ESP32 settings
SAMPLE_RATE = 16000
CHANNELS = 1
SAMPLE_WIDTH = 2  # 2 bytes = 16-bit resolution

@app.websocket("/ws/audio-stream")
async def audio_stream_endpoint(websocket: WebSocket):
    await websocket.accept()
    print("ESP32 Connected via WebSocket")
    
    # Create a unique filename based on timestamp
    filename = f"recording_{int(time.time())}.wav"
    
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
                
                # Write to WAV file
                wav_file.writeframes(audio_data)
                
    except WebSocketDisconnect:
        print("ESP32 Disconnected - Recording Saved")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    # Run with: python server.py
    # Listen on 0.0.0.0 to allow access from external devices (ESP32)
    print("Lunar Audio Streaming Server is running...")
    uvicorn.run(app, host="0.0.0.0", port=8000)