import edge_tts
import asyncio
import subprocess
import os

VOICE = "en-US-AriaNeural"

async def tts(text: str, wav_path: str):
    mp3_path = "temp.mp3"

    communicate = edge_tts.Communicate(text, VOICE)
    await communicate.save(mp3_path)

    # Convert MP3 → WAV (16kHz, mono, 16-bit PCM)
    subprocess.run([
        "ffmpeg", "-y",
        "-i", mp3_path,
        "-ar", "16000",
        "-ac", "1",
        "-sample_fmt", "s16",
        wav_path
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    os.remove(mp3_path)

    print(f"TTS complete: {wav_path}")