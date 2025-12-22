#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <driver/i2s.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ================= WIFI =================
const char* ssid = "Yahya";
const char* password = "taj@1727";
const char* SERVER_URL = "http://192.168.0.107:8000/";

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= I2S =================
#define I2S_PORT I2S_NUM_0

// I2S Speaker (MAX98357A)
#define SPK_LRC  27
#define SPK_BCLK 14
#define SPK_DIN  32

// OLED
#define OLED_SDA 21
#define OLED_SCL 22


#define AUDIO_BUFFER_SIZE 1024
uint8_t audioBuffer[AUDIO_BUFFER_SIZE];

// =====================================================

void setupWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void setupOLED() {
  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Ready");
  display.display();
}

void showText(String text) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("AI:");
  display.println(text);
  display.display();
}

void setupI2SSpeaker() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 512,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = SPK_BCLK,
    .ws_io_num = SPK_LRC,
    .data_out_num = SPK_DIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
}

// =====================================================

void playWavStream(WiFiClient* stream) {
  uint8_t header[44];
  stream->read(header, 44);  // skip WAV header

  size_t bytesRead;
  while (stream->connected()) {
    bytesRead = stream->read(audioBuffer, AUDIO_BUFFER_SIZE);
    if (bytesRead > 0) {
      size_t bytesWritten;
      i2s_write(I2S_PORT, audioBuffer, bytesRead, &bytesWritten, portMAX_DELAY);
    } else {
      break;
    }
  }
}

// =====================================================

void sendPrompt(String prompt) {
  HTTPClient http;
  WiFiClient client;

  http.begin(client, SERVER_URL);
  http.addHeader("Content-Type", "text/plain");

  int httpCode = http.POST(prompt);
  Serial.print("HTTP Code: ");
  Serial.println(httpCode);

  if (httpCode <= 0) {
    Serial.println("Connection failed");
    http.end();
    return;
  }

  WiFiClient* stream = http.getStreamPtr();

  // ---------- READ JSON ----------
  String jsonText = "";
  while (stream->connected()) {
    String line = stream->readStringUntil('\n');
    if (line.indexOf("Content-Type: application/json") >= 0) {
      stream->readStringUntil('\n'); // empty line
      jsonText = stream->readStringUntil('\r');
      break;
    }
  }

  StaticJsonDocument<256> doc;
  deserializeJson(doc, jsonText);
  String aiText = doc["text"].as<String>();

  Serial.println("AI: " + aiText);
  showText(aiText);

  // ---------- FIND AUDIO ----------
  while (stream->connected()) {
    String line = stream->readStringUntil('\n');
    if (line.indexOf("Content-Type: audio/wav") >= 0) {
      stream->readStringUntil('\n');
      playWavStream(stream);
      break;
    }
  }

  http.end();
}

// =====================================================

void setup() {
  Serial.begin(115200);
  setupWiFi();
  setupOLED();
  setupI2SSpeaker();

  Serial.println("Type prompt and press ENTER");
}

void loop() {
  // if (Serial.available()) {
  //   String prompt = Serial.readStringUntil('\n');
  //   prompt.trim();
  //   if (prompt.length() > 0) {
  //     sendPrompt(prompt);
  //   }
  // }
  HTTPClient http;
  WiFiClient client;

  http.begin(client, SERVER_URL);
  http.addHeader("Content-Type", "text/plain");

  int httpCode = http.GET();
  Serial.print("HTTP Code: ");
  Serial.println(httpCode); 
  delay(5000);
}
