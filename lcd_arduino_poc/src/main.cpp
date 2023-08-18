#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <math.h>
#include <stdio.h>

/* Constructor */
U8G2_ST7571_128X128_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 12, /* data=*/ 13, /* cs=*/ 1, /* dc=*/ 3, /* reset=*/ 2);

const double rotation_angle = atan(double(0.3333));
const double camera_to_lcd_pixels_scale = 0.5;
const double camera_to_lcd_pixels_x_offset = 10;
const double camera_to_lcd_pixels_y_offset = -10;

void setup() {
  // put your setup code here, to run once:
  u8g2.begin();
  u8g2.setContrast(200); // good enough for straight on. Trade off "off" transparency with "on" darkness
  // put your main code here, to run repeatedly:
  u8g2.firstPage();
  do {
    u8g2.drawBox(0,0,25,25);
  } while ( u8g2.nextPage() );

  delay(100);
  Serial.begin(115200);
  Serial.setTimeout(100); //ms
}

// TODO: Test with lower voltage? Probably can't with this...

// Normally we'd take in an angle and do some math, but for fun let's
// see how far off just a scale + offset will be

// Expected data from camera is: "123 -456 10; 789 012 34"
// 123 = x offset in pixels from top left
// 456 = y offset in pixels from top left
// 10 = size in pixels of object to block
// ";", then any additional circles to show, ended by "\n"


uint8_t numLights = 0;
#define MAX_LIGHTS 10

// 15 degrees
// Using Div by 1024 as it can be done with right shifts
// .9659
#define COS_ANGLE 989/1024
// .2588
#define SIN_ANGLE 265/1024
#define SCALE 1


// denoted in LCD pixels (corrected values)
struct Light
{
  uint16_t xOffset;
  uint16_t yOffset;
  uint16_t radius;
};

struct Light lights[MAX_LIGHTS];

char tempStr[15];
uint8_t i = 0;
char * strtokIndex = 0;
uint16_t cameraX = 0;
uint16_t cameraY = 0;
uint16_t cameraRadius = 0;

void loop() {
  if (!Serial.available()) {
    return;
  }

  // Collect data until a semicolon or newline
  tempStr[i] = Serial.read();
  i++;

  if (tempStr[i] == ';') {
    strtokIndex = strtok(tempStr, " ");
    cameraX = atoi(tempStr);
    strtokIndex = strtok(NULL, " ");
    cameraY = atoi(strtokIndex);
    strtokIndex = strtok(NULL, "\n");
    cameraRadius = atoi(strtokIndex);

    // Transform x,y to x',y' via scale and rotation...
    // Won't be this simple probably
    lights[numLights].xOffset = (cameraX*COS_ANGLE - cameraY*SIN_ANGLE)*SCALE;
    lights[numLights].yOffset = (cameraX*SIN_ANGLE + cameraY*COS_ANGLE)*SCALE;
    lights[numLights].radius = cameraRadius*SCALE;
    i = 0;
    numLights++;
  } else if (numLights >= MAX_LIGHTS || tempStr[i] == '\n') {
    // Draw on LCD, reuse i
    do {
      for (i = 0; i < numLights; i++) {
        u8g2.drawCircle(lights[i].xOffset, lights[i].yOffset, lights[i].radius);
      }
    } while ( u8g2.nextPage() );

    i = 0;
    numLights = 0;
  }




  /*
  char foo[10];
  for (int i = 0; i < 256; i+= 5) {
    u8g2.setContrast(i);
    delay(100);
    u8g2.clear();
    u8g2.firstPage();
    sprintf(foo, "%i", i);
    do {
    u8g2.drawStr(0,20,foo);
     } while ( u8g2.nextPage() );
  }
  */

}