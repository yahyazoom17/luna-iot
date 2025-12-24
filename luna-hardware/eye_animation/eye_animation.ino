#include "LunaEyes.h"

LunaEyes luna;

void setup() {
    Serial.begin(9600);
    luna.begin();
}

void loop() {
    luna.blink();
    delay(1000);

    luna.happy();
    delay(3000);

    luna.sad();
    delay(3000);

    luna.thinking(2);
    delay(3000);

    luna.sleep();
    delay(5000);
}