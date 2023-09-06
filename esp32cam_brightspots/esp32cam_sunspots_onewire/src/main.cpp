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
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(config.pixel_format == PIXFORMAT_JPEG){
    if(psramFound()){
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if(config.pixel_format == PIXFORMAT_JPEG){
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

// Setup LED FLash if LED pin is defined in camera_pins.h
#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif

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
  uint16 x1;
  uint16 y1;
  uint16 x2;
  uint16 y2;
} BoundingBox;


boolean WithinBoundingBox (BoundingBox *b, uint16 expandBorder, uint16 x, uint16 y) {
  return (x >= b->x1 - expandBorder &&
          y >= b->y1 - expandBorder &&
          x <  b->x2 + expandBorder && 
          y <  b->y2 + expandBorder);    
}

void ExpandBoundingBox (BoundingBox *b, uint16 x, uint16 y) {
  if (x < b->x1) b->x1 = x;
  if (y < b->y1) b->y1 = y;
  if (y > b->x2) b->x2 = x;
  if (y > b->y2) b->y2 = y;
}

#define MAX_BOXES 20
BoundingBox boxes[MAX_BOXES];
uint8 numBoxes = 0;
int8 containedInBoundingBoxIndex = -1;
void loop() {
  // Do nothing. Everything is done in another task by the web server
  delay(10000);

  /*
  // Would like manual control of exposure, probably need to go manual or do some more research on the OpenMV API
  // Although....maybe auto exposure is close to how eye would work? For a simple algorithm. Worth testing in a dark and light room and with some low background light like your car headlights pointed out.
  res = s->set_ae_level(s, val);

  // Ask for just grayscale and no JPEG compression
  set_pixformat(s PIXFORMAT_GRAYSCALE);

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      if (!(im[x][y] > THRESHOLD)} {
        continue;
      }

      containedInBoundingBoxIndex = -1;
      for (int i = 0; i < numBoxes; i++) {
        // If not within expanded border of this bounding box or
        // within current bounding box, there's nothing to do
        if (!WithinBoundingBox(&boxes[i], 1, x, y) || 
             WithinBoundingBox(&boxes[i], 0, x, y)) {
          continue;
        }

        // Expand current box
        ExpandBoundingBox(&boxes[i], x, y);

        if (containedInBoundingBoxIndex == -1) {
          containedInBoundingBoxIndex = i;
          continue;
        }

        // Combine bounding boxes and remove current box
        ExpandBoundingBox(&boxes[containedInBoundingBoxIndex], boxes[i].x1, boxes[i].y1);
        ExpandBoundingBox(&boxes[containedInBoundingBoxIndex], boxes[i].x2, boxes[i].y2);
        for (; i < numBoxes; i++) {
          boxes[i-1] = boxes[i];
        }
      }

      if (containedInBoundingBoxIndex == -1) {
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

  // Write out to ... which serial?
  for (int i = 0; i < numBoxes; i++) {
    Serial.print("%d %d %d %d; ", boxes[i].x1, boxes[i].y1, boxes[i].x2, boxes[i].y2);
  }
  Serial.println();
  
  */
}