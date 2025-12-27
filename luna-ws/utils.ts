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

/**
 * Converts any audio file to a 16kHz, Mono, 16-bit WAV suitable for ESP32 streaming.
 * @param {string} inputPath - Path to your source wav (e.g., 'final_voice.wav')
 * @param {string} outputPath - Path for the converted file
 */
export function prepareAudioForEsp32(inputPath: string, outputPath: string) {
  // -ar 16000: Set sample rate to 16kHz
  // -ac 1: Set to 1 audio channel (mono)
  // -sample_fmt s16: Set to 16-bit signed integer
  const command = `ffmpeg -i "${inputPath}" -ar 16000 -ac 1 -sample_fmt s16 "${outputPath}" -y`;

  exec(command, (error) => {
    if (error) {
      console.error(`Conversion failed: ${error.message}`);
      return;
    }
    console.log(`Audio ready for ESP32: ${outputPath}`);
  });
}

prepareAudioForEsp32(
  "./recordings/harvard.wav",
  "./recordings/harvard_16k_mono.wav"
);
