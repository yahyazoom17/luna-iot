#include <WiFi.h>
#include <ArduinoJson.h>
#include <driver/i2s.h>
#include <LittleFS.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// --- Audio Libraries ---
#include "AudioFileSourceHTTPStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

// --- Configuration ---
const char* ssid = "Yahya";
const char* password = "taj@1727";
const char* server_host = "192.168.0.107"; 
const int server_port = 8000;

// --- Pin Definitions ---
// Microphone -> Uses I2S_NUM_0
#define I2S_MIC_PORT I2S_NUM_0
#define I2S_MIC_SCK 14
#define I2S_MIC_WS  18
#define I2S_MIC_SD  32

// Speaker -> Uses I2S_NUM_1
#define I2S_SPK_PORT_NUM 1  // Library uses int 0 or 1
#define I2S_SPK_BCLK 26
#define I2S_SPK_LRC  25
#define I2S_SPK_DIN  22

// --- Settings ---
#define CHUNK_SIZE 512
#define VAD_THRESHOLD 1000  
#define MAX_RECORD_TIME 5000 

// --- Globals ---
WiFiClient client;
bool is_recording = false;
unsigned long last_voice_time = 0;
unsigned long record_start_time = 0;
uint8_t chunk_buffer[CHUNK_SIZE]; 

// Audio Objects
AudioGeneratorMP3 *mp3 = NULL;
AudioFileSourceHTTPStream *file = NULL;
AudioFileSourceBuffer *buff = NULL;
AudioOutputI2S *out = NULL;

void setup() {
  // 1. DISABLE BROWNOUT DETECTOR (Prevents Reboot Loop)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  delay(1000); 
  Serial.println("\n--- ESP32 Audio Assistant ---");

  // 2. Init WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  // 3. Init Microphone Driver (Port 0)
  installMic();
  Serial.println("System Ready. Listening...");
}

void loop() {
  // --- PLAYBACK MODE (Uses Port 1) ---
  if (mp3 && mp3->isRunning()) {
    if (!mp3->loop()) { 
      // Song finished
      mp3->stop(); 
      cleanupPlayback(); 
      Serial.println("Playback Finished. Resuming Listening...");
      installMic(); // Re-enable Port 0
    }
    return; 
  }

  // --- LISTENING MODE (Uses Port 0) ---
  size_t bytes_read;
  // Read from I2S_MIC_PORT (Port 0)
  i2s_read(I2S_MIC_PORT, chunk_buffer, CHUNK_SIZE, &bytes_read, 0);

  if (bytes_read > 0) {
    // Analyze Audio Volume
    int16_t* samples = (int16_t*)chunk_buffer;
    int32_t max_val = 0;
    for (int i = 0; i < bytes_read / 2; i++) {
       int32_t val = abs(samples[i]);
       if(val > max_val) max_val = val;
    }

    // A. Detect Voice to Start
    if (!is_recording && max_val > VAD_THRESHOLD) {
      startStreaming();
    }
    // B. Stream Audio if Recording
    else if (is_recording) {
      streamChunk(chunk_buffer, bytes_read);
      
      if (max_val > VAD_THRESHOLD) last_voice_time = millis();
      
      if (millis() - last_voice_time > 1000 || millis() - record_start_time > MAX_RECORD_TIME) {
        finishStreaming();
      }
    }
  }
}

// ==========================================
//           NETWORK STREAMING
// ==========================================

void startStreaming() {
  Serial.println(">> Voice Detected! Connecting to Server...");
  
  if (!client.connect(server_host, server_port)) {
    Serial.println("!! Connection Failed");
    return;
  }

  client.println("POST /stream_audio HTTP/1.1");
  client.println("Host: " + String(server_host));
  client.println("Content-Type: application/octet-stream");
  client.println("Transfer-Encoding: chunked");
  client.println("Connection: close");
  client.println();

  is_recording = true;
  record_start_time = millis();
  last_voice_time = millis();
}

void streamChunk(uint8_t* data, size_t len) {
  if (client.connected()) {
    client.print(len, HEX);
    client.println();
    client.write(data, len);
    client.println();
  }
}

void finishStreaming() {
  Serial.println(">> Recording Done. Waiting for Response...");
  is_recording = false;

  if (client.connected()) {
    client.println("0");
    client.println();
    
    long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) { 
        Serial.println("!! Timeout waiting for server"); 
        client.stop(); 
        return; 
      }
    }

    String jsonResponse = "";
    bool headerEnded = false;
    while(client.available()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") headerEnded = true; 
      if (headerEnded) jsonResponse += line;
    }
    client.stop(); 

    int jsonStart = jsonResponse.indexOf('{');
    if (jsonStart != -1) {
      String json = jsonResponse.substring(jsonStart);
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, json);
      
      String stream_url = doc["stream_url"].as<String>();
      String message = doc["message"].as<String>();
      
      Serial.println("AI Says: " + message);

      // Switch to Playback (Port 1)
      uninstallMic(); // Free up RAM from Port 0
      String full_url = "http://" + String(server_host) + ":" + String(server_port) + stream_url;
      playStream(full_url);
    }
  }
}

// ==========================================
//            AUDIO DRIVER LOGIC
// ==========================================

void installMic() {
  // CONFIG FOR I2S PORT 0
  i2s_config_t rec_cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4, 
    .dma_buf_len = 512,
    .use_apll = false
  };
  i2s_pin_config_t rec_pin = { 
    .bck_io_num = I2S_MIC_SCK, 
    .ws_io_num = I2S_MIC_WS, 
    .data_out_num = -1, 
    .data_in_num = I2S_MIC_SD 
  };
  // Install Driver on PORT 0
  i2s_driver_install(I2S_MIC_PORT, &rec_cfg, 0, NULL);
  i2s_set_pin(I2S_MIC_PORT, &rec_pin);
}

void uninstallMic() {
  // Uninstall PORT 0
  i2s_driver_uninstall(I2S_MIC_PORT);
}

void playStream(String url) {
  if (mp3) cleanupPlayback();
  
  Serial.println("Playing: " + url);
  file = new AudioFileSourceHTTPStream(url.c_str());
  buff = new AudioFileSourceBuffer(file, 2048); 
  
  // CONFIG FOR I2S PORT 1
  // The '1' argument forces the library to use I2S_NUM_1
  out = new AudioOutputI2S(I2S_SPK_PORT_NUM); 
  out->SetPinout(I2S_SPK_BCLK, I2S_SPK_LRC, I2S_SPK_DIN);
  
  mp3 = new AudioGeneratorMP3();
  mp3->begin(buff, out);
}

void cleanupPlayback() {
  if (mp3) { delete mp3; mp3 = NULL; }
  if (buff) { delete buff; buff = NULL; }
  if (file) { delete file; file = NULL; }
  if (out) { delete out; out = NULL; }
}