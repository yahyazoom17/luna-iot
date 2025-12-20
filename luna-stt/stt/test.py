import whisper
import torch

print(torch.__version__)
print(torch.version.cuda)
print(torch.cuda.is_available())

model = whisper.load_model("turbo", device="cuda")
result = model.transcribe("10229.mp3", fp16=False, language="ta")
print(result["text"])