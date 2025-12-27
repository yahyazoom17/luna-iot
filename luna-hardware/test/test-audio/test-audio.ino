#include <WiFi.h>
#include <WebSocketsClient.h>
#include <driver/i2s.h>

// --- Configuration ---
const char* ssid = "Yahya";
const char* password = "taj@1727";
const char* ws_host = "192.168.0.107"; // Your computer's IP
const int ws_port = 3001;

// --- VAD (Voice Activity Detection) Settings ---
const int32_t NOISE_THRESHOLD = 500; 
const unsigned long SILENCE_TIMEOUT = 1000; // 1 second
unsigned long lastVoiceTime = 0;
bool isStreaming = false;

WebSocketsClient webSocket;

// --- I2S Pin Mapping (Based on your Provided Config) ---
#define MIC_I2S_SCK 14  // SCK pin
#define MIC_I2S_WS  18  // WS pin
#define MIC_I2S_SD  32  // SD pin

#define SAMPLE_RATE 16000
#define DMA_BUF_LEN 512 

void setupI2S() {
  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    // Using MSB alignment to ensure digital sync and reduce noise
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = DMA_BUF_LEN,
    .use_apll = false
  };

  const i2s_pin_config_t pin_config = {
    .mck_io_num = I2S_PIN_NO_CHANGE,
    .bck_io_num = MIC_I2S_SCK,
    .ws_io_num = MIC_I2S_WS,
    .data_out_num = -1, // Not used for mic
    .data_in_num = MIC_I2S_SD
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WSc] Disconnected!");
      break;
    case WStype_CONNECTED:
      Serial.println("[WSc] Connected to Server");
      break;
    case WStype_TEXT:
      Serial.printf("[WSc] Server Response: %s\n", payload);
      break;
    case WStype_BIN:
      // Binary data handling would go here if receiving audio
      break;
  }
}

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  setupI2S();

  webSocket.begin(ws_host, ws_port, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();

  if (webSocket.isConnected()) {
    int16_t buffer[DMA_BUF_LEN];
    size_t bytes_read;
    
    // Read raw audio from I2S Port 0
    i2s_read(I2S_NUM_0, buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);

    if (bytes_read > 0) {
      // 1. Digital Gain Boost (Helps avoid quiet noise/vibration issues)
      for (int i = 0; i < bytes_read / 2; i++) {
        buffer[i] = buffer[i] << 3; // 8x volume boost
      }

      // 2. Simple VAD: Check Peak Amplitude
      int32_t max_sample = 0;
      for (int i = 0; i < bytes_read / 2; i++) {
        int32_t abs_val = abs(buffer[i]);
        if (abs_val > max_sample) max_sample = abs_val;
      }

      // 3. Logic to Start/Stop Stream based on Voice
      if (max_sample > NOISE_THRESHOLD) {
        if (!isStreaming) {
          Serial.println("Voice Detected: Streaming...");
          webSocket.sendTXT("START_NEW_RECORDING"); // Signal server
        }
        lastVoiceTime = millis();
        isStreaming = true;
      }

      if (isStreaming) {
        webSocket.sendBIN((uint8_t*)buffer, bytes_read);

        // Auto-stop after 1 second of silence
        if (millis() - lastVoiceTime > SILENCE_TIMEOUT) {
          Serial.println("Silence: Stream Stopped.");
          isStreaming = false;
          webSocket.sendTXT("STOP_RECORDING"); 
          i2s_zero_dma_buffer(I2S_NUM_0); // Flush buffer
        }
      }
    }
  }
}