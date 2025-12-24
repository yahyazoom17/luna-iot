import { exec } from "child_process";

/**
 * Converts raw PCM to WAV
 * @param {string} inputRawPath - Path to the .raw file
 * @param {string} outputWavPath - Path for the resulting .wav file
 */
export function convertRawToWav(inputRawPath: string, outputWavPath: string) {
  // -f s16le: format signed 16-bit little endian
  // -ar 16000: 16kHz sample rate
  // -ac 1: 1 channel (mono)
  const command = `ffmpeg -f s16le -ar 16000 -ac 1 -i "${inputRawPath}" -y "${outputWavPath}"`;

  exec(command, (error) => {
    if (error) {
      console.error(`Conversion Error: ${error.message}`);
      return;
    }
    console.log(`Success! Audio saved to: ${outputWavPath}`);
  });
}

// Example usage:
// Run this when ws.on('close') is triggered
// convertRawToWav('./streamed_audio.raw', './final_recording.wav');
