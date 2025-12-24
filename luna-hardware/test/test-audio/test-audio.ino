#include <driver/i2s.h>
#include <math.h>

#define I2S_NUM      I2S_NUM_1
#define SPK_BCLK     26
#define SPK_LRC      25
#define SPK_DIN      22

void setup() {
  Serial.begin(115200);

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64
  };

  i2s_pin_config_t pin_config = {
    .mck_io_num = I2S_PIN_NO_CHANGE,
    .bck_io_num = SPK_BCLK,
    .ws_io_num = SPK_LRC,
    .data_out_num = SPK_DIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM, &pin_config);
}

void loop() {
  // Generate a simple 440Hz Sine Wave
  static float frequency = 440.0;
  static float phase = 0;
  float phase_increment = 2.0 * PI * frequency / 16000.0;

  int16_t sample;
  size_t bytes_written;

  for (int i = 0; i < 64; i++) {
    sample = (int16_t)(sin(phase) * 10000); // 10000 is the volume amplitude
    i2s_write(I2S_NUM, &sample, sizeof(sample), &bytes_written, portMAX_DELAY);
    phase += phase_increment;
    if (phase >= 2.0 * PI) phase -= 2.0 * PI;
  }
}