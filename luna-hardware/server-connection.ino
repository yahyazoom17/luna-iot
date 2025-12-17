#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Yahya";
const char* password = "taj@1727";

// Your Next.js API URL
// Example: http://192.168.1.100:3000/api/esp32
const char* serverUrl = "http://192.168.0.108:3000/api/device/status";

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
}

int getWiFiStrength() {
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.RSSI(); // returns dBm
  }
  return 0;
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

//     StaticJsonDocument<200> doc;
// doc["deviceId"] = "luna-001";
// doc["batteryLevel"] = 80;
// doc["wifiStrength"] = getWiFiStrength();

// String jsonData;
// serializeJson(doc, jsonData);

// Serial.println(jsonData);

    String jsonData = "{";
jsonData += "\"deviceId\":\"luna-001\",";
jsonData += "\"batteryLevel\":80,";
jsonData += "\"wifiStrength\":" + String(getWiFiStrength());
jsonData += "}";

    int httpResponseCode = http.PUT(jsonData);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response: " + response);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }

  delay(10000); // send every 10 seconds
}
