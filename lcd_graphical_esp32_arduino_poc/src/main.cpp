#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <math.h>
#include <stdio.h>
#include <HardwareSerial.h>

#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

/* Constructor */
U8G2_ST7571_128X128_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 12, /* data=*/ 13, /* cs=*/ 1, /* dc=*/ 3, /* reset=*/ 2);

// Right side from viewers perspective
#define LCD_RIGHT


// TODO: Test with lower voltage? Probably can't with this...

// Normally we'd take in an angle and do some math, but for fun let's
// see how far off just a scale + offset will be

// Expected data from camera is: "10 20 \n 50 50 \n"
// 10 = x offset in pixels from top left of first light
// 20 = y offset in pixels from top left of first light
// 50 = x ... 2nd light
// 50 = y ... 2nd light
#define MILLIS_PER_DRAW (1000/30)
#define LIGHT_RADIUS 6
const char ExpectedStringChars[] = "000 000 ";
#define STR_BUFFER_LENGTH (sizeof(ExpectedStringChars) + 1)

uint8_t numLights = 0;
#define MAX_LIGHTS 15

// denoted in LCD pixels (corrected values)
struct Light
{
  uint16_t x1;
  uint16_t y1;
  uint8_t radius;
};

struct Light lights[MAX_LIGHTS];

// FIXME to be camera actual resolution
static const uint16_t cam_width = 256;
static const uint16_t cam_height = 256;
static const uint16_t lcd_width = 128;
static const uint16_t lcd_height = 128;
static const uint16_t scale = cam_height/lcd_height; // Very rough (integer) guess for now, probably will need fractional



HardwareSerial Serial_UART(0);

void CameraToLCD(uint16_t *x, uint16_t *y)
{
  int32_t xTemp = *x + 20;
  int32_t yTemp = *y + 80;
/*
  // Transform x,y to x',y' via scale and rotation...
  // Won't be this simple probably
  int32_t xTemp2 = (xTemp*COS_ANGLE - yTemp*SIN_ANGLE)/scale;
  int32_t yTemp2 = (xTemp*SIN_ANGLE + yTemp*COS_ANGLE)/scale;
  xTemp = xTemp2;
  yTemp = yTemp2;
  */

/*
  Trying to do fast dewarping...
    * Transform to (r,theta) space with a center.
      * Center determination could be with 4 corner points
      * Warping on (r) should be independent of theta, so just need to gather dewarping points on one radial and compute dewarping for r and not theta.
      * Could just do a few piecewise linear interpolations, or maybe add spline
      * Pre-compute a 2D lookup table in x and y (based on (r,theta)) and then do linear interpolation in between the points in x and y. That'll work
      * Or if you need to compute real r, can do r^2 only and not sqrt. Or look up efficient hypot like these: https://stackoverflow.com/questions/3506404/fast-hypotenuse-algorithm-for-embedded-processor
 
*/

  //yTemp = yTemp;
  // Invert left to right
  xTemp = lcd_width - xTemp;

  *x = (uint16_t)xTemp;
  *y = (uint16_t)yTemp;
}

void setup() {
  // put your setup code here, to run once:
  u8g2.begin();
#ifdef LCD_RIGHT
    u8g2.setContrast(175); // good enough for straight on. Trade off "off" transparency with "on" darkness
#else
    u8g2.setContrast(200);
#endif
  u8g2.clearDisplay();
  u8g2.firstPage();
  do {
#ifdef LCD_RIGHT
    u8g2.drawDisc(60,60,LIGHT_RADIUS);
#else
    u8g2.drawBox(20,85,20,20);
#endif

  } while ( u8g2.nextPage() );

  Serial.begin(921600);
  Serial.setTimeout(100); //ms
  Serial.println("Startup");

  Serial_UART.begin(921600);
  delay(500);
  //u8g2.clear();
  delay(500);
}

unsigned long timeStartDraw = 0;
void drawLightsOnDisplay() {
  // Currently 100ms to draw...seems too much. Weird!
  uint16_t i = 0;
  uint16_t j = 0;
  timeStartDraw = millis();

  u8g2.clearBuffer();
  Serial.printf("Clear buf: %d\n", millis() - timeStartDraw);
  u8g2.firstPage();
  Serial.printf("First page: %d\n", millis() - timeStartDraw);

  // Draw on LCD
  do {
    for (i = 0; i < numLights; i++) {
      // A bit hacky, but I need rotated squares for now
      u8g2.drawDisc(lights[i].x1, lights[i].y1, lights[i].radius);
    }
  } while ( u8g2.nextPage() );
  Serial.printf("Last page: %d\n", millis() - timeStartDraw);
  numLights = 0;
}


char lightSerial[STR_BUFFER_LENGTH];
uint8_t lightSerialIndex = 0;

void ResetString() {
  lightSerialIndex = 0;
  memset(lightSerial, 0, STR_BUFFER_LENGTH);
}

uint8_t i = 0;
void loop() {

  char * token;
  uint16_t numBytesToRead = Serial_UART.available();
  //Serial.printf("ToRead: %d, Index: %d\n", numBytesToRead, lightSerialIndex);

  if (numBytesToRead == 0) {
    if (numLights > 0) {
      //Serial.println("Drawing");


      drawLightsOnDisplay();
      //numLights = 0;



    }
    return;
  }

  // TODO: Turn into an event instead of loop? (sleep in between)

  while (1) {
    if (lightSerialIndex >= STR_BUFFER_LENGTH) {
      // Probably garbage when first plugging in. Let's reset the string
      Serial.printf("lightSerialIndex >= %d\n", STR_BUFFER_LENGTH);
      ResetString();
      return;
    }

    if (Serial_UART.peek() == '\n') {
      // Consume newline but don't use it
      Serial_UART.read();
      break;
    }

    Serial_UART.read(&lightSerial[lightSerialIndex], 1);
    lightSerialIndex++;
    numBytesToRead--;

    if (numBytesToRead == 0) {
      return;
    }
  }
  // Now we have line previously terminated by a newline character

  // Validate string matches expected string chars
  for (i = 0; i < sizeof(ExpectedStringChars); i++) {
    if ((ExpectedStringChars[i] == '0' && (lightSerial[i] > '9' || lightSerial[i] < '0')) ||
        (ExpectedStringChars[i] == ' ' && lightSerial[i] != ' ')) {
      ResetString();
      return;
    }
  }

  Serial.println("Parsing string");

  // So now we have a valid line. Let's parse it!
  // Add null just in case
  lightSerial[STR_BUFFER_LENGTH-1] = '\0';
  //Serial.printf("lightSerial: %s\n", lightSerial);

  // Only process one "light" and return through the loop
  token = strtok(lightSerial, " ");
  lights[numLights].x1 = atoi(token);
  token = strtok(NULL, " ");
  lights[numLights].y1 = atoi(token);
  lights[numLights].radius = LIGHT_RADIUS;
  Serial.printf("Light_cam: %d %d\n", lights[numLights].x1, lights[numLights].y1 );
  CameraToLCD(&lights[numLights].x1, &lights[numLights].y1);
  Serial.printf("Light_lcd: %d %d\n", lights[numLights].x1, lights[numLights].y1 );
  numLights++;

  ResetString();
}