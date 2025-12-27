import whisper
import torch

print(torch.__version__)
print(torch.version.cuda)
print(torch.cuda.is_available())

model = whisper.load_model("turbo", device="cuda")

def transcribe_audio(file_path: str) -> str:
    result = model.transcribe(file_path, fp16=False, language="en")
    return result["text"]