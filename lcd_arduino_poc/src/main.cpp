#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <math.h>
#include <stdio.h>

/* Constructor */
U8G2_ST7571_128X128_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 12, /* data=*/ 13, /* cs=*/ 1, /* dc=*/ 3, /* reset=*/ 2);

#define LCD_NUM 1

void setup() {
  // put your setup code here, to run once:
  u8g2.begin();
  if (LCD_NUM == 0) {
    u8g2.setContrast(175); // good enough for straight on. Trade off "off" transparency with "on" darkness
  } else {
    u8g2.setContrast(200);
  }
  // put your main code here, to run repeatedly:
  u8g2.firstPage();
  do {
    if (LCD_NUM == 0) {
      u8g2.drawBox(20,85,20,20);
    } else {
      u8g2.drawBox(85,85,20,20);
    }
  } while ( u8g2.nextPage() );

  delay(100);
  Serial.begin(921600);
  Serial.setTimeout(100); //ms
  Serial.println("Startup");
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

// 30 degrees
// Using Div by 1024 as it can be done with right shifts
// .86602
#define COS_ANGLE 887/1024
// .5
#define SIN_ANGLE 512/1024
#define SCALE 1


// denoted in LCD pixels (corrected values)
struct Light
{
  uint8_t xOffset;
  uint8_t yOffset;
  uint8_t radius;
};

struct Light lights[MAX_LIGHTS];

char tempStr[15];
uint8_t i = 0;
char * strtokIndex = 0;
uint8_t cameraX = 0;
uint8_t cameraY = 0;
uint8_t cameraRadius = 0;

void loop() {
  if (!Serial.available()) {
    return;
  }

  // Collect data until a semicolon or newline
  tempStr[i] = Serial.read();
  i++;
  Serial.println(tempStr);
  Serial.println(numLights);
  Serial.println(i);
  

  if (tempStr[i-1] == ';' || tempStr[i-1] == 'd') {
    strtokIndex = strtok(tempStr, " ");
    cameraX = atoi(tempStr);
    strtokIndex = strtok(NULL, " ");
    cameraY = atoi(strtokIndex);
    strtokIndex = strtok(NULL, ";");
    cameraRadius = atoi(strtokIndex);
    Serial.println("Got one!");

    // Transform x,y to x',y' via scale and rotation...
    // Won't be this simple probably
    lights[numLights].xOffset = (cameraX*COS_ANGLE - cameraY*SIN_ANGLE)*SCALE;
    lights[numLights].yOffset = (cameraX*SIN_ANGLE + cameraY*COS_ANGLE)*SCALE;
    lights[numLights].radius = cameraRadius*SCALE;
    i = 0;
    numLights++;
    Serial.println(numLights);
  } else if (numLights >= MAX_LIGHTS || tempStr[i-1] == '\r' || tempStr[i-1] == '\n') {
    Serial.println("Drawing!");
    Serial.println(numLights);
    u8g2.clearDisplay();


    // Draw on LCD, reuse i
    do {
      for (i = 0; i < numLights; i++) {
        u8g2.drawDisc(lights[i].xOffset, lights[i].yOffset, lights[i].radius, U8G2_DRAW_ALL);
      }
    } while ( u8g2.nextPage() );

    i = 0;
    numLights = 0;
  }

}