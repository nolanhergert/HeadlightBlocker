#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <math.h>
#include <stdio.h>
#include <HardwareSerial.h>

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
// 50 = y ... 2nd llight

#define MILLIS_PER_DRAW (1000/30)
#define LIGHT_RADIUS 6
#define MIN_NUM_CHARS 10

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

  Serial.print(*x);
  Serial.print(", ");
  Serial.println(*y);
  // Rotate about center of view
  int32_t xTemp = *x;
  int32_t yTemp = *y;

  // Transform x,y to x',y' via scale and rotation...
  // Won't be this simple probably
  int32_t xTemp2 = (xTemp*COS_ANGLE - yTemp*SIN_ANGLE)/scale;
  int32_t yTemp2 = (xTemp*SIN_ANGLE + yTemp*COS_ANGLE)/scale;
  xTemp = xTemp2;
  yTemp = yTemp2;

  //yTemp = yTemp;
  // Invert left to right
  xTemp = lcd_width - xTemp;

  *x = (uint16_t)xTemp;
  *y = (uint16_t)yTemp + 40;

  Serial.print(xTemp);
  Serial.print(", ");
  Serial.println(yTemp);
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
    u8g2.drawDisc(60,60,10);
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

void drawLightsOnDisplay() {
  uint16_t i = 0;
  uint16_t j = 0;

  u8g2.clearBuffer();
  u8g2.firstPage();

  // Draw on LCD
  do {
    for (i = 0; i < numLights; i++) {
      // A bit hacky, but I need rotated squares for now
      u8g2.drawDisc(lights[i].x1, lights[i].y1, lights[i].radius);
    }
  } while ( u8g2.nextPage() );
  numLights = 0;
}


unsigned long previousDrawMillis = 0;
void loop() {

  char lightSerial[25];
  uint8_t lightSerialIndex = 0;
  char * token;


  if (millis() - previousDrawMillis > MILLIS_PER_DRAW) {
    drawLightsOnDisplay();
    previousDrawMillis = millis();
  }

  // TODO: Turn into an event instead of loop
  uint16_t numBytesToRead = Serial_UART.available();
  // Want at least one total sequence
  if (numBytesToRead < MIN_NUM_CHARS) {
    return;
  }

  // The below is probably not that efficient (reading one byte at a time)
  // but it is simple and readable!

  // Get a known starting sequence, the previous light's newline.
  // Unlikely to happen other than at startup
  while (Serial_UART.peek() != '\n') {
    Serial_UART.read();
  }
  // Just in case when testing I forget the starting newline...
  numBytesToRead = Serial_UART.available();
  if (numBytesToRead < MIN_NUM_CHARS) {
    return;
  }

  // But don't want this newline, skip over it
  Serial_UART.read();

  // Read up to but not including the next newline
  while (Serial_UART.peek() != '\n') {
    lightSerial[lightSerialIndex] = Serial_UART.read();
    lightSerialIndex++;
  }
  lightSerial[lightSerialIndex] = '\0';
  lightSerialIndex++;

  //Serial.printf("lightSerial: %s\n", lightSerial);

  // Only process one "light" and return through the loop
  token = strtok(lightSerial, " ");
  lights[numLights].x1 = atoi(token);
  token = strtok(NULL, " ");
  lights[numLights].y1 = atoi(token);
  lights[numLights].radius = LIGHT_RADIUS;
  // Serial.printf("Light: %d %d\n", lights[numLights].x1, lights[numLights].y1 );
  CameraToLCD(&lights[numLights].x1, &lights[numLights].y1);
  numLights++;
}