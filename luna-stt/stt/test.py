import whisper

model = whisper.load_model("turbo")
result = model.transcribe("harvard.wav", fp16=False, language="en")
print(result["text"])