#include "esp_camera.h"
#include <WiFi.h>

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Face Recognition is DISABLED for ESP32 and ESP32-S2, because it takes up from 15
//            seconds to process single frame. Face Detection is ENABLED if PSRAM is enabled as well

// ===================
// Select camera model
// ===================
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
//#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD
//#define CAMERA_MODEL_DFRobot_FireBeetle2_ESP32S3 // Has PSRAM
//#define CAMERA_MODEL_DFRobot_Romeo_ESP32S3 // Has PSRAM
#include "camera_pins.h"

// ===========================
// Enter your WiFi credentials
// ===========================
const char* ssid = "**********";
const char* password = "**********";

void startCameraServer();
void setupLedFlash(int pin);

void setup() {
  Serial.begin(921600);
  Serial.setDebugOutput(true);
  Serial.println();

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

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");




}
typedef struct BoundingBox_ {
  uint16_t x1;
  uint16_t y1;
  uint16_t x2;
  uint16_t y2;
} BoundingBox;

#define THRESHOLD (254)

// Does the bounding box (plus an optional border expansion) contain point (x,y)?
boolean WithinBoundingBox (BoundingBox *b, uint16_t expandBorder, uint16_t x, uint16_t y) {
  return (x >= b->x1 - expandBorder &&
          y >= b->y1 - expandBorder &&
          x <  b->x2 + expandBorder &&
          y <  b->y2 + expandBorder);
}

// Expand a given bounding box to contain a new point (x,y)
void ExpandBoundingBox (BoundingBox *b, uint16_t x, uint16_t y) {
  if (x < b->x1) b->x1 = x;
  if (y < b->y1) b->y1 = y;
  if (x > b->x2) b->x2 = x;
  if (y > b->y2) b->y2 = y;
}

#define MAX_BOXES 20
BoundingBox boxes[MAX_BOXES];
uint8_t numBoxes = 0;
int8_t containedInBoundingBox = -1;
uint32_t countWhitePixels = 0;
void loop() {
  camera_fb_t *fb = NULL;
  // Do nothing. Everything is done in another task by the web server
  delay(10000);

/*
  fb = esp_camera_fb_get();

  countWhitePixels = 0;
  for (int x = 0; x < fb->width; x++) {
    for (int y = 0; y < fb->height; y++) {
      if (fb->buf[x + y*fb->width] < THRESHOLD) {
        continue;
      }

      countWhitePixels++;
    }
  }
  Serial.println(countWhitePixels);


      // We found a bright pixel! Add it to an existing bounding box
      // or make a new one
      containedInBoundingBox = -1;
      for (int i = 0; i < numBoxes; i++) {
        // If not within expanded border of this bounding box or
        // within current bounding box, there's nothing to do
        if (!WithinBoundingBox(&boxes[i], 1, x, y) ||
             WithinBoundingBox(&boxes[i], 0, x, y)) {
          continue;
        }

        // Expand current box
        ExpandBoundingBox(&boxes[i], x, y);

        if (containedInBoundingBox == -1) {
          containedInBoundingBox = i;
          continue;
        }

        // Overlapping bounding boxes. Combine bounding boxes and remove current box
        ExpandBoundingBox(&boxes[containedInBoundingBox], boxes[i].x1, boxes[i].y1);
        ExpandBoundingBox(&boxes[containedInBoundingBox], boxes[i].x2, boxes[i].y2);
        for (; i < numBoxes; i++) {
          boxes[i-1] = boxes[i];
        }
      }

      if (containedInBoundingBox == -1) {
        // Create new box
        boxes[i].x1 = x;
        boxes[i].y1 = y;
        boxes[i].x2 = x;
        boxes[i].y2 = y;
        numBoxes++;
        assert(numBoxes < MAX_BOXES);
      }
    }
  }


  // TODO: Write out to web gui?

  // Write out to ... which serial?
  for (int i = 0; i < numBoxes; i++) {
    Serial.print("%d %d %d %d; ", boxes[i].x1, boxes[i].y1, boxes[i].x2, boxes[i].y2);
  }
  Serial.println();

*/
}