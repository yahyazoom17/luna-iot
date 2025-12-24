#include <stdint.h>
#ifndef LUNA_EYES_H
#define LUNA_EYES_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class LunaEyes {
public:
    LunaEyes();

    void begin();
    void center();
    void blink(int speed = 12);
    void sleep();
    void wakeup();
    void happy();
    void sad();
    void angry();
    void moveRightBig();
    void moveLeftBig();
    void thinking(int cycles);
    void demo(uint8_t index);

private:
    void draw(bool update = true);
    void moveBigEye(int direction);

    // Display
    Adafruit_SSD1306 display;

    // Eye states
    int ref_eye_height = 40;
    int ref_eye_width  = 40;
    int ref_space_between_eye = 10;
    int ref_corner_radius = 10;

    int left_eye_x, left_eye_y;
    int right_eye_x, right_eye_y;
    int left_eye_width, left_eye_height;
    int right_eye_width, right_eye_height;
};

#endif
