import { WebSocketServer } from "ws";
import fs from "fs";
import WebSocket from "ws";

const wss = new WebSocketServer({ port: 3001 });

wss.on("connection", (ws) => {
  console.log("ESP32 connected for playback");

  ws.on("message", (data) => {
    if (data.toString() === "PLAY_LUNA") {
      streamWavToEsp32(ws, "./recordings/harvard.wav");
    }
  });
});

function streamWavToEsp32(ws: WebSocket, filePath: string) {
  // We skip the 44-byte WAV header to send only raw PCM data
  const readStream = fs.createReadStream(filePath, {
    start: 44,
    highWaterMark: 1024,
  });

  console.log("Streaming audio...");

  readStream.on("data", (chunk) => {
    // Pause the stream briefly to match the 16kHz sample rate
    // (16000 samples/sec * 2 bytes/sample = 32000 bytes/sec)
    readStream.pause();
    ws.send(chunk, { binary: true });

    // 1024 bytes is approx 32ms of audio at 16kHz
    setTimeout(() => {
      if (ws.readyState === 1) readStream.resume();
    }, 30);
  });

  readStream.on("end", () => {
    console.log("Playback finished.");
    ws.send("PLAYBACK_COMPLETE");
  });
}
