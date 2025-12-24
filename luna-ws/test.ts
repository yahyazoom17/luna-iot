import { WebSocketServer } from "ws";
import fs from "fs";
import WebSocket from "ws";

const wss = new WebSocketServer({ port: 3001 });

wss.on("connection", (ws) => {
  console.log("ESP32 Connected. Ready to stream audio.");

  ws.on("message", (data) => {
    if (data.toString() === "GET_AUDIO") {
      streamAudioFile(ws, "./recordings/harvard.wav");
    }
  });
});

function streamAudioFile(ws: WebSocket, filePath: string) {
  const stream = fs.createReadStream(filePath, { highWaterMark: 1024 }); // Read in 1KB chunks

  console.log("Streaming started...");

  stream.on("data", (chunk) => {
    // We pause the stream briefly to prevent buffer overflow on the ESP32
    // For 16kHz Mono 16-bit, we need ~32KB per second.
    stream.pause();
    ws.send(chunk, { binary: true });

    setTimeout(() => {
      stream.resume();
    }, 25); // Approximately 32ms of audio per 1024 byte chunk
  });

  stream.on("end", () => {
    console.log("Streaming finished.");
    ws.send("STREAM_FINISHED");
  });
}
