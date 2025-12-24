import { WebSocketServer, WebSocket } from "ws";
import fs from "fs";
import path from "path";
import { exec } from "child_process";

// Ensure the recordings directory exists
const RECORDINGS_DIR = path.join(process.cwd(), "recordings");
if (!fs.existsSync(RECORDINGS_DIR)) {
  fs.mkdirSync(RECORDINGS_DIR, { recursive: true });
}

/**
 * Converts raw PCM to WAV using FFmpeg
 */
export function convertRawToWav(inputRawPath: string, outputWavPath: string) {
  // -f s16le: 16-bit little endian | -ar 16000: 16kHz | -ac 1: Mono
  const command = `ffmpeg -f s16le -ar 16000 -ac 1 -i "${inputRawPath}" -y "${outputWavPath}"`;

  exec(command, (error) => {
    if (error) {
      console.error(`[FFMPEG ERROR]: ${error.message}`);
      return;
    }
    console.log(`[CONVERSION SUCCESS]: ${outputWavPath}`);

    // Optional: Delete the .raw file after successful conversion to save space
    // fs.unlinkSync(inputRawPath);
  });
}

const wss = new WebSocketServer({ port: 3001, path: "/" });

wss.on("connection", (ws: WebSocket, req) => {
  const clientIp = req.socket.remoteAddress;
  console.log(`New Client: ${clientIp}. Total: ${wss.clients.size}`);

  // This variable is unique to THIS specific connection
  let currentFileStream: fs.WriteStream | null = null;
  let currentRawPath: string = "";

  ws.on("message", (data: Buffer, isBinary: boolean) => {
    // --- 1. HANDLE BINARY AUDIO DATA ---
    if (isBinary) {
      if (currentFileStream && currentFileStream.writable) {
        currentFileStream.write(data);
      }
      return;
    }

    // --- 2. HANDLE STRING COMMANDS ---
    const msg = data.toString().trim();

    if (msg === "START_NEW_RECORDING") {
      const filename = `final_voice.raw`;
      currentRawPath = path.join(RECORDINGS_DIR, filename);

      console.log(`[REC START]: Creating ${filename}`);

      // Close previous if it exists
      if (currentFileStream) currentFileStream.end();

      currentFileStream = fs.createWriteStream(currentRawPath);
    } else if (msg === "STOP_RECORDING") {
      if (currentFileStream) {
        console.log("[REC STOP]: Finalizing audio...");
        const outputWav = currentRawPath.replace(".raw", ".wav");

        currentFileStream.end(() => {
          convertRawToWav(currentRawPath, outputWav);
          currentFileStream = null;
        });
      }
    }

    // --- 3. BROADCAST TEXT MESSAGES TO DASHBOARD ---
    else {
      wss.clients.forEach((client) => {
        if (client !== ws && client.readyState === WebSocket.OPEN) {
          client.send(msg);
        }
      });
    }
  });

  ws.on("close", () => {
    console.log(`Client ${clientIp} disconnected.`);

    // Emergency cleanup: If client disconnects while recording
    if (currentFileStream) {
      const outputWav = currentRawPath.replace(".raw", ".wav");
      currentFileStream.end(() => {
        convertRawToWav(currentRawPath, outputWav);
      });
    }
  });
});

console.log("WebSocket server is running on ws://localhost:3001");
