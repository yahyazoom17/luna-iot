#include <WiFi.h>
#include <WebSocketsClient.h>
#include <driver/i2s.h>

const char* ssid = "Yahya";
const char* password = "taj@1727";
const char* ws_host = "192.168.0.107"; // Your computer's IP
const int ws_port = 3001;

// --- VAD Settings ---
const int32_t NOISE_THRESHOLD = 500; // Adjust this based on your room noise
const unsigned long SILENCE_TIMEOUT = 1000; // 1 seconds of silence to stop
unsigned long lastVoiceTime = 0;
bool isStreaming = false;

WebSocketsClient webSocket;

// I2S Config
#define I2S_WS 18
#define I2S_SD 32
#define I2S_SCK 14
#define SAMPLE_RATE 16000
#define DMA_BUF_LEN 512  // Smaller buffer for lower latency

void setupI2S() {
  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = DMA_BUF_LEN,
    .use_apll = false
  };

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
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
      Serial.println("[WSc] Connected to url: " + String((char*)payload));
      break;
    case WStype_TEXT:
      Serial.printf("[WSc] get text: %s\n", payload);
      break;
    case WStype_BIN:
      // We don't expect to receive binary, only send it
      break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  setupI2S();

  // FIX 2: WebSocket initialization
  webSocket.begin(ws_host, ws_port, "/");
  webSocket.onEvent(webSocketEvent); // This will now recognize the function
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();

  if (webSocket.isConnected()) {
    int16_t i2s_buffer[DMA_BUF_LEN];
    size_t bytes_read;
    
    // Always read from I2S to keep the hardware buffer from overflowing
    i2s_read(I2S_NUM_0, i2s_buffer, sizeof(i2s_buffer), &bytes_read, portMAX_DELAY);

    if (bytes_read > 0) {
      // 1. Check for Voice Activity (VAD)
      int32_t max_sample = 0;
      for (int i = 0; i < bytes_read / 2; i++) {
        int32_t abs_val = abs(i2s_buffer[i]);
        if (abs_val > max_sample) max_sample = abs_val;
      }

      // 2. State Management
      if (max_sample > NOISE_THRESHOLD) {
        if (!isStreaming) {
          Serial.println("Voice detected: Starting NEW stream");
          webSocket.sendTXT("START_NEW_RECORDING"); // Tell server to create a fresh file
        }
        lastVoiceTime = millis();
        isStreaming = true;
      }

      // 3. Only Send Data if isStreaming is TRUE
      if (isStreaming) {
        webSocket.sendBIN((uint8_t*)i2s_buffer, bytes_read);

        // 4. Handle Stop Condition
        if (millis() - lastVoiceTime > SILENCE_TIMEOUT) {
          Serial.println("Silence: Stopping stream and clearing buffers.");
          isStreaming = false;
          
          // CRITICAL: Tell server to close the current file
          webSocket.sendTXT("STOP_RECORDING"); 
          
          // CRITICAL: Flush the hardware buffer so no "old" audio lingers
          i2s_zero_dma_buffer(I2S_NUM_0); 
        }
      }
    }
  }
}