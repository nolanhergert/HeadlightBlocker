
#include "esp_camera.h"         // https://github.com/espressif/esp32-camera
#include <Arduino.h>

#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

// Initialize camera to...


#include <C:\Users\10PRO\esp\esp-idf\components\esp32-camera\target\private_include\ll_cam.h>
#include <C:\Users\10PRO\esp\esp-idf\components\esp32-camera\sensors\private_include\ov2640_regs.h>


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
  config.xclk_freq_hz = 10000000; // Matters! 24Mhz is finicky...
  config.frame_size = FRAMESIZE_QQVGA; //FRAMESIZE_QQVGA Matters!
  // Crashes when in jpeg
  config.pixel_format = PIXFORMAT_GRAYSCALE; // for easier/faster processing. Done on CPU side by skipping over UV of YUV
  // JPEG wants frame size of 4:3 to work. However, even on 160X120 (QQVGA) it's still a max of only 25 FPS
  config.grab_mode = CAMERA_GRAB_LATEST; // doesn't matter
  config.fb_location = CAMERA_FB_IN_DRAM; // surprisingly doesn't matter, dram vs psram
  config.jpeg_quality = 63;
  config.fb_count = 2; // Try one instead of 2?

  // Try CAMERA_FB_IN_PSRAM and CIF again. Maybe it's not actually transferring

  log_printf("Startup! %d\n", millis());
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  log_printf("Startup 2 (avoidable)! %d\n", millis());
  sensor_t * s = esp_camera_sensor_get();
  // Would like manual control of exposure, probably need to go manual or do some more research on the OpenMV API
  // Although....maybe auto exposure is close to how eye would work? For a simple algorithm. Worth testing in a dark and light room and with some low background light like your car headlights pointed out.
  s->set_exposure_ctrl(s, 0);
  s->set_aec_value(s, 0);
  // Play with lower CLK divisions. Default is 0x3 in normal app
  // Could change this in the ov2640_settings.h startup sequence, ov2640_settings_to_cif
  // 50 FPS! But get this error:
  s->set_reg(s, 0x100 | CLKRC, 0x3F, 0x0);

  //s->set_reg(s, 0x100 | REG32, 0xFF, REG32_SVGA); // Turn off clock divider


  //  WRITE_REG_OR_RETURN(BANK_SENSOR, CLKRC, c.clk);
  ///  WRITE_REG_OR_RETURN(BANK_DSP, R_DVP_SP, c.pclk);
  //log_printf("CLKRC: 0x%x\n", s->get_reg(s, CLKRC, 0xFFFFFFFF));
  //log_printf("BD50: 0x%x\n", s->get_reg(s, BD50, 0xFFFFFFFF));



}

unsigned long startMillis = 0;
void loop() {
  startMillis = millis();
  //log_printf("Start: %d\n", millis());
  camera_fb_t *fb = NULL;
  // Gets latest frame in buffer
  fb = esp_camera_fb_get();
  if (!fb) {
    //log_printf("Image not found!\n");
  } else {
    //log_printf("Captured frame!\n");
  }
  //log_printf("Cam mid: %d\n", millis());

  esp_camera_fb_return(fb);
  //log_printf("Cam end: %d\n", millis());

  //DBG_PIN_SET(0);

  //vTaskDelay(50 / portTICK_RATE_MS);

  //log_printf("Frame\n");


  uint16_t firstWhitePixelX = 0;
  uint16_t firstWhitePixelY = 0;
  uint16_t x, y = 0;
  for (x = 0; x < fb->width; x++) {
    for (y = 0; y < fb->height; y++) {
      if (fb->buf[x + y*fb->width] == 255) {
        firstWhitePixelX = x;
        firstWhitePixelY = y;
        log_printf("%03d %03d \n", firstWhitePixelX, firstWhitePixelY);

        goto BreakLoop;
      }
    }
  }

BreakLoop:
  //log_printf("MS: %d, FPS: %d\n", (millis() - startMillis), 1000/(millis() - startMillis));

  return;






  // Another thing to test is startup time after full power off
  // 200ms startup time. Good to know!
}
