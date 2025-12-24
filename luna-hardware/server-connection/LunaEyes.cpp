#include "LunaEyes.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

LunaEyes::LunaEyes()
: display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET)
{}

void LunaEyes::begin() {
    Wire.begin(21, 22);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        while (1);
    }

    center();
}

void LunaEyes::draw(bool update) {
    display.clearDisplay();

    display.fillRoundRect(
        left_eye_x - left_eye_width / 2,
        left_eye_y - left_eye_height / 2,
        left_eye_width,
        left_eye_height,
        ref_corner_radius,
        SSD1306_WHITE
    );

    display.fillRoundRect(
        right_eye_x - right_eye_width / 2,
        right_eye_y - right_eye_height / 2,
        right_eye_width,
        right_eye_height,
        ref_corner_radius,
        SSD1306_WHITE
    );

    if (update) display.display();
}

void LunaEyes::center() {
    left_eye_width = right_eye_width = ref_eye_width;
    left_eye_height = right_eye_height = ref_eye_height;

    left_eye_x = SCREEN_WIDTH / 2 - ref_eye_width / 2 - ref_space_between_eye / 2;
    right_eye_x = SCREEN_WIDTH / 2 + ref_eye_width / 2 + ref_space_between_eye / 2;

    left_eye_y = right_eye_y = SCREEN_HEIGHT / 2;

    draw(true);
}

void LunaEyes::blink(int speed) {
    for (int i = 0; i < 3; i++) {
        left_eye_height -= speed;
        right_eye_height -= speed;
        draw();
        delay(2);
    }
    for (int i = 0; i < 3; i++) {
        left_eye_height += speed;
        right_eye_height += speed;
        draw();
        delay(2);
    }
}

void LunaEyes::sleep() {
    left_eye_height = right_eye_height = 2;
    draw(true);
}

void LunaEyes::wakeup() {
    sleep();
    for (int h = 2; h <= ref_eye_height; h += 2) {
        left_eye_height = right_eye_height = h;
        draw(true);
        delay(10);
    }
}

void LunaEyes::happy() {
    center();
    delay(300);
}

void LunaEyes::sad() {
    center();
    delay(300);
}

void LunaEyes::angry() {
    center();
    delay(300);
}

void LunaEyes::moveRightBig() { moveBigEye(1); }
void LunaEyes::moveLeftBig()  { moveBigEye(-1); }

void LunaEyes::moveBigEye(int direction) {
    for (int i = 0; i < 5; i++) {
        left_eye_x += direction * 2;
        right_eye_x += direction * 2;
        draw();
        delay(5);
    }
    center();
}

void LunaEyes::thinking(int cycles = 3) {
  int move_amp = 6;     // how far eyes move
  int squint = 6;       // eye height reduction

  for (int c = 0; c < cycles; c++)
  {
    // 👀 Look left
    left_eye_x  -= move_amp;
    right_eye_x -= move_amp;
    left_eye_height  -= squint;
    right_eye_height -= squint;
    draw(true);
    delay(200);

    // 👀 Look right
    left_eye_x  += move_amp * 2;
    right_eye_x += move_amp * 2;
    draw(true);
    delay(200);

    // 👀 Back to center
    left_eye_x  -= move_amp;
    right_eye_x -= move_amp;
    left_eye_height  += squint;
    right_eye_height += squint;
    draw(true);
    delay(250);

    // 😐 subtle blink (thinking pause)
    blink(8);
    delay(150);
  }
  center();
}

void LunaEyes::demo(uint8_t index) {
    switch (index) {
        case 0: wakeup(); break;
        case 1: center(); break;
        case 2: moveRightBig(); break;
        case 3: moveLeftBig(); break;
        case 4: blink(12); break;
        case 5: happy(); break;
        case 6: sad(); break;
        case 7: angry(); break;
        case 8: sleep(); break;
    }
}
