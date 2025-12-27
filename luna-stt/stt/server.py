from fastapi import FastAPI, UploadFile
import json
from fastapi.responses import StreamingResponse
import os
from dotenv import load_dotenv
from pydantic import BaseModel
import requests
from tts import tts

load_dotenv()

app = FastAPI()

OPENROUTER_API_KEY = os.getenv("OPENROUTER_API_KEY")

OPENROUTER_URL = "https://openrouter.ai/api/v1/chat/completions"

HEADERS = {
    "Authorization": f"Bearer {OPENROUTER_API_KEY}",
    "Content-Type": "application/json",
    "HTTP-Referer": "http://localhost",
    "X-Title": "ESP32 Voice Assistant"
}

SYSTEM_PROMPT = "You are Luna, a smart and friendly IoT voice assistant."

class ChatModel(BaseModel):
    prompt: str

@app.get("/")
async def root():
    return {"message": "Hello World, from LUNA!"}

@app.post("/chat")
async def ask_model(data: ChatModel):
    prompt = data
    payload = {
        "model": "meta-llama/llama-3.3-70b-instruct:free",
        "messages": [
            {"role": "system", "content": SYSTEM_PROMPT},
            {"role": "user", "content": prompt}
        ],
        "temperature": 0.7,
        "max_tokens": 50
    }

    response = requests.post(
        OPENROUTER_URL,
        headers=HEADERS,
        json=payload,
        timeout=60
    )

    response.raise_for_status()

    data = response.json()
    print(data)
    await tts(data["choices"][0]["message"]["content"], "output.wav")
    return data["choices"][0]["message"]["content"]

@app.post("/audio")
async def voice_ai():
    input_wav = "input.wav"
    output_wav = "response.wav"

    # with open(input_wav, "wb") as f:
    #     f.write(await file.read())

    user_text = "Hello! I'm Luna, your friendly IoT voice assistant."  # or Whisper STT
    ai_text = await ask_model(user_text)
    print(f"AI Response: {ai_text}")
    await tts(ai_text, output_wav)

    # def generate():
    #     # ---- PART 1: JSON TEXT ----
    #     json_data = json.dumps({
    #         "text": ai_text
    #     }).encode()

    #     yield b"--frame\r\n"
    #     yield b"Content-Type: application/json\r\n\r\n"
    #     yield json_data + b"\r\n"

    #     # ---- PART 2: AUDIO WAV ----
    #     with open(output_wav, "rb") as audio:
    #         yield b"--frame\r\n"
    #         yield b"Content-Type: audio/wav\r\n\r\n"
    #         yield audio.read() + b"\r\n"

    #     yield b"--frame--"

    # return StreamingResponse(
    #     generate(),
    #     media_type="multipart/form-data; boundary=frame"
    # )

    return {"message": "Audio response generated.", "audio_file": output_wav, "text": ai_text}