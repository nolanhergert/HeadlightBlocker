
#include "esp_camera.h"         // https://github.com/espressif/esp32-camera
#include <Arduino.h>

#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

// Initialize camera to...



//
void setup() {
  Serial.begin(921600);


  camera_config_t config;
   config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_GRAYSCALE; // for processing
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }


  sensor_t * s = esp_camera_sensor_get();
  // Would like manual control of exposure, probably need to go manual or do some more research on the OpenMV API
  // Although....maybe auto exposure is close to how eye would work? For a simple algorithm. Worth testing in a dark and light room and with some low background light like your car headlights pointed out.
  s->set_ae_level(s, -2);

}

void loop() {
  camera_fb_t *fb = NULL;
  // Gets latest frame in buffer
  fb = esp_camera_fb_get();
  if (!fb) {
    //log_printf("Image not found!\n");
  } else {
    //log_printf("Captured frame!\n");
  }

  esp_camera_fb_return(fb);
  vTaskDelay(250 / portTICK_RATE_MS);



  uint16_t firstWhitePixelX = 0;
  uint16_t firstWhitePixelY = 0;
  uint16_t x, y = 0;
  for (x = 0; x < fb->width; x++) {
    for (y = 0; y < fb->height; y++) {
      if (fb->buf[x + y*fb->width] == 255) {
        firstWhitePixelX = x;
        firstWhitePixelY = y;
        log_printf("%d %d;\n", firstWhitePixelX, firstWhitePixelY);
        goto BreakLoop;
      }
    }
  }
BreakLoop:
  return;



}
