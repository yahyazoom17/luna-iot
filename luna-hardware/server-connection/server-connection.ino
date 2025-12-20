#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>

const char* ssid = "Yahya";
const char* password = "taj@1727";

// Your Next.js API URL
// Example: http://192.168.1.100:3000/api/esp32
const char* serverUrl = "http://192.168.0.104:3000/api/device/status";

WebSocketsClient webSocket;

bool wsConnected = false;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_CONNECTED:
      Serial.println("✅ WebSocket connected");
      wsConnected = true;
      break;

    case WStype_DISCONNECTED:
      Serial.println("❌ WebSocket disconnected");
      wsConnected = false;
      break;

    case WStype_TEXT:
      Serial.print("Received from server: ");
      Serial.println((char*)payload);
      break;
  }
}

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  delay(1000);
  webSocket.begin("192.168.0.104", 3001, "/");
  webSocket.onEvent(webSocketEvent);
}

int getWiFiStrength() {
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.RSSI(); // returns dBm
  }
  return 0;
}

void loop() {
  webSocket.loop();

  static unsigned long lastSend = 0;

  if (wsConnected && millis() - lastSend > 1000) {
    lastSend = millis();

    StaticJsonDocument<200> doc;
    doc["deviceId"] = "luna-001";
    doc["batteryLevel"] = 80;
    doc["wifiStrength"] = getWiFiStrength();
    doc["isConnected"] = true;

    String jsonData;
    serializeJson(doc, jsonData);

    bool sent = webSocket.sendTXT(jsonData);
    Serial.println(sent ? "📤 Sent Device Status OK" : "📛 Sending Device Status FAILED");
  }   

  // if (WiFi.status() == WL_CONNECTED) {

  //   HTTPClient http;
  //   http.begin(serverUrl);
  //   http.addHeader("Content-Type", "application/json");

  // StaticJsonDocument<200> doc;
  // doc["deviceId"] = "luna-001";
  // doc["batteryLevel"] = 80;
  // doc["wifiStrength"] = getWiFiStrength();

  // String jsonData;
  // serializeJson(doc, jsonData);

  // Serial.println(jsonData);

  // int httpResponseCode = http.PUT(jsonData);

  // Serial.print("HTTP Response code: ");
  // Serial.println(httpResponseCode);

  //   if (httpResponseCode > 0) {
  //     String response = http.getString();
  //     Serial.println("Response: " + response);
  //   }

  //   http.end();
  // } else {
  //   Serial.println("WiFi not connected");
  // }
}
