void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n--- Memory Check ---");
  
  // Total internal RAM (usually ~320KB)
  Serial.printf("Internal Free Heap: %d bytes\n", ESP.getFreeHeap());

  // Check for External PSRAM (usually 4MB or 8MB if present)
  if (psramInit()) {
    Serial.println("PSRAM Found and Initialized!");
    Serial.printf("Total PSRAM: %d bytes\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM:  %d bytes\n", ESP.getFreePsram());
  } else {
    Serial.println("No PSRAM detected.");
  }
}

void loop() {}