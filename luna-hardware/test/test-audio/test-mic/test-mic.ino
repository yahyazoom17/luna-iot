#include <WiFi.h>
#include <WebSocketsClient.h>
#include <driver/i2s.h>

// --- Network Config ---
const char* ssid = "Yahya";
const char* password = "taj@1727";
const char* websocket_server_host = "192.168.0.107"; // REPLACE with your PC's IP
const uint16_t websocket_server_port = 8000;
const char* websocket_server_path = "/ws/audio-stream";

// --- I2S Microphone Config (INMP441) ---
#define I2S_SCK 14
#define I2S_WS  18
#define I2S_SD  32
#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 16000
#define BUFFER_LEN 512

WebSocketsClient webSocket;

// I2S Configuration
void i2s_install() {
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // INMP441 sends 24 bits in 32-bit slot
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  // L/R pin grounded
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_LEN,
    .use_apll = false
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
}

void i2s_setpin() {
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };
  i2s_set_pin(I2S_PORT, &pin_config);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WSc] Disconnected!");
      break;
    case WStype_CONNECTED:
      Serial.println("[WSc] Connected to server");
      break;
    case WStype_TEXT:
    case WStype_BIN:
      break;
  }
}

void setup() {
  Serial.begin(115200);

  // 1. Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  // 2. Setup WebSocket
  webSocket.begin(websocket_server_host, websocket_server_port, websocket_server_path);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);

  // 3. Setup I2S
  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);
}

void loop() {
  webSocket.loop();

  if(webSocket.isConnected()) {
    // Buffer to read 32-bit raw data from I2S
    int32_t raw_samples[BUFFER_LEN];
    size_t bytes_read = 0;

    // Read from I2S
    esp_err_t result = i2s_read(I2S_PORT, &raw_samples, sizeof(raw_samples), &bytes_read, portMAX_DELAY);

    if (result == ESP_OK && bytes_read > 0) {
      int samples_read = bytes_read / 4; // 4 bytes per sample (32-bit)
      
      // Create a smaller buffer for 16-bit PCM to send over Network
      int16_t pcm_samples[samples_read];

      for (int i = 0; i < samples_read; i++) {
        // INMP441 data is 24-bit, left-aligned in the 32-bit word.
        // We shift right by 16 to get the most significant 16 bits.
        // You can use >> 14 or >> 15 to increase digital gain if it's too quiet.
        pcm_samples[i] = (int16_t)(raw_samples[i] >> 16); 
      }

      // Send binary data to FastAPI
      webSocket.sendTXT("bytes");
      webSocket.sendBIN((uint8_t*)pcm_samples, samples_read * 2);
    }
  }
}