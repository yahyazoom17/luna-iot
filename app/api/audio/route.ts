import { NextResponse, NextRequest } from "next/server";
import fs from "fs";
import path from "path";

// Configure to handle large file uploads
export const config = {
  api: {
    bodyParser: false,
  },
};

export async function POST(request: NextRequest) {
  try {
    console.log("=".repeat(50));
    console.log("Received audio upload request");
    console.log("Headers:", Object.fromEntries(request.headers));

    // Read the raw binary data
    const buffer = await request.arrayBuffer();
    const audioData = Buffer.from(buffer);

    console.log(`Audio data received: ${audioData.length} bytes`);

    if (audioData.length === 0) {
      console.log("ERROR: No audio data received!");
      return NextResponse.json(
        { status: "error", message: "No audio data" },
        { status: 400 }
      );
    }

    // Save as WAV file
    const timestamp = new Date().toISOString().replace(/[:.]/g, "-");
    const filename = `audio_${timestamp}.wav`;
    const uploadsDir = path.join(process.cwd(), "public", "uploads");

    // Create uploads directory if it doesn't exist
    if (!fs.existsSync(uploadsDir)) {
      fs.mkdirSync(uploadsDir, { recursive: true });
    }

    const filepath = path.join(uploadsDir, filename);

    // WAV file header configuration
    const sampleRate = 16000;
    const channels = 1;
    const bitsPerSample = 16;
    const byteRate = sampleRate * channels * (bitsPerSample / 8);
    const blockAlign = channels * (bitsPerSample / 8);
    const dataSize = audioData.length;
    const fileSize = 36 + dataSize;

    // Create WAV header
    const header = Buffer.alloc(44);

    // RIFF header
    header.write("RIFF", 0);
    header.writeUInt32LE(fileSize, 4);
    header.write("WAVE", 8);

    // fmt chunk
    header.write("fmt ", 12);
    header.writeUInt32LE(16, 16); // fmt chunk size
    header.writeUInt16LE(1, 20); // audio format (1 = PCM)
    header.writeUInt16LE(channels, 22);
    header.writeUInt32LE(sampleRate, 24);
    header.writeUInt32LE(byteRate, 28);
    header.writeUInt16LE(blockAlign, 32);
    header.writeUInt16LE(bitsPerSample, 34);

    // data chunk
    header.write("data", 36);
    header.writeUInt32LE(dataSize, 40);

    // Combine header and audio data
    const wavFile = Buffer.concat([header, audioData]);

    // Write to file
    fs.writeFileSync(filepath, wavFile);

    console.log(`Audio saved as ${filename}`);
    console.log("=".repeat(50));

    const duration = dataSize / (sampleRate * channels * (bitsPerSample / 8));

    return NextResponse.json({
      status: "success",
      message: "Audio received and saved",
      filename: filename,
      size_bytes: dataSize,
      duration_seconds: duration.toFixed(2),
      url: `/uploads/${filename}`,
    });
  } catch (error) {
    console.error("Error processing audio:", error);
    return NextResponse.json(
      { status: "error", message: error.message },
      { status: 500 }
    );
  }
}

export async function GET() {
  return NextResponse.json({
    message: "ESP32 Audio Server is running",
    endpoint: "/api/upload-audio",
    method: "POST",
  });
}
