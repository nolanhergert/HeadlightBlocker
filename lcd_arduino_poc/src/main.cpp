#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <math.h>
#include <stdio.h>
#include <HardwareSerial.h>

/* Constructor */
U8G2_ST7571_128X128_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 12, /* data=*/ 13, /* cs=*/ 1, /* dc=*/ 3, /* reset=*/ 2);

// Right side from viewers perspective
//#define LCD_RIGHT


// TODO: Test with lower voltage? Probably can't with this...

// Normally we'd take in an angle and do some math, but for fun let's
// see how far off just a scale + offset will be

// Expected data from camera is: "10 20 10; 50 50 10"
// 10 = x offset in pixels from top left
// 20 = y offset in pixels from top left
// 10 = radius in pixels of object to block
// ";", then any additional circles to show, ended by "\n"


uint8_t numLights = 0;
#define MAX_LIGHTS 10

// FIXME to be camera actual dimensions
#define CAM_HEIGHT 128
#define CAM_WIDTH 128
#define LCD_HEIGHT 128
#define LCD_WIDTH  128


HardwareSerial Serial_UART(0);

// 23.3 degrees
// Using Div by 1024 as it can be done with right shifts
// Google: cos(23.3 degrees)*1024
#define COS_ANGLE 940/1024
#ifdef LCD_RIGHT
#define SIN_ANGLE 405/1024
#else
#define SIN_ANGLE -405/1024
#endif
#define SCALE 1



// denoted in LCD pixels (corrected values)
struct Light
{
  uint16_t x1;
  uint16_t y1;
  uint16_t x2;
  uint16_t y2;
  uint8_t radius;
};

struct Light lights[MAX_LIGHTS];

char tempStr[15];
uint8_t i = 0;
uint8_t j = 0;
char * strtokIndex = 0;
uint8_t cameraX = 0;
uint8_t cameraY = 0;
uint8_t cameraRadius = 0;



void CameraToLCD(uint16_t *x, uint16_t *y) 
{
  // Rotate about center of view
  int32_t xTemp = *x - CAM_WIDTH/2;
  int32_t yTemp = *y - CAM_HEIGHT/2;

  // Transform x,y to x',y' via scale and rotation...
  // Won't be this simple probably
  int32_t xTemp2 = (xTemp*COS_ANGLE - yTemp*SIN_ANGLE)*SCALE;
  int32_t yTemp2 = (xTemp*SIN_ANGLE + yTemp*COS_ANGLE)*SCALE;
  xTemp = xTemp2 + LCD_WIDTH/2;
  yTemp = yTemp2 + LCD_HEIGHT/2;

  // Invert as the LCDs are upside down technically
  yTemp = LCD_HEIGHT - yTemp;

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
  // put your main code here, to run repeatedly:
  u8g2.firstPage();
  do {
#ifdef LCD_RIGHT
    u8g2.drawBox(0,85,8,8);
    u8g2.drawBox(15,85,10,10);
    u8g2.drawBox(35,85,12,12);
    u8g2.drawBox(55,85,14,14);
    u8g2.drawBox(75,85,16,16);
#else
    u8g2.drawBox(20,85,20,20);
#endif

  } while ( u8g2.nextPage() );

  Serial.begin(921600);
  Serial.setTimeout(100); //ms
  Serial.println("Startup");

  Serial_UART.begin(921600);
  delay(500);
  u8g2.clear();
  delay(500);
}

void loop() {
  if (!Serial.available() && !Serial_UART.available()) {
    return;
  }

  if (Serial.available()) {
    // Collect data until a semicolon or newline
    tempStr[i] = Serial.read();
    // Forward message from USB onto other display
    Serial_UART.print(tempStr[i]);
  } else {
    // Collect data until a semicolon or newline
    tempStr[i] = Serial_UART.read();
  }

  i++;
  Serial.println(tempStr);
  Serial.println(numLights);
  Serial.println(i);
  

  if (tempStr[i-1] == ';') {
    strtokIndex = strtok(tempStr, " ");
    lights[numLights].x1 = atoi(tempStr);
    strtokIndex = strtok(NULL, " ");
    lights[numLights].y1 = atoi(strtokIndex);
      strtokIndex = strtok(tempStr, " ");
    lights[numLights].x2 = atoi(tempStr);
    strtokIndex = strtok(NULL, ";");
    lights[numLights].y2 = atoi(strtokIndex);
    Serial.println("Got one!");

    CameraToLCD(&lights[numLights].x1, &lights[numLights].y1);
    CameraToLCD(&lights[numLights].x2, &lights[numLights].y2);

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
        u8g2.drawBox(lights[i].x1, lights[i].y1, lights[i].x2, lights[i].y2);
      }
    } while ( u8g2.nextPage() );

    i = 0;
    numLights = 0;
  }
  
  
/*
  struct Light light;
  do {
    for (i = 0; i < 5; i++) {
      for (j = 0; j < 5; j++) {
        // TODO: Not quite done yet, not right values.
        // But need to test...
        light.x1 = 80 + i*10;
        light.y1 = 80 + j*10;
        light.x2 = 80 + i*20;
        light.y2 = 80 + j*20;
        CameraToLCD(&lights[numLights].x1, &lights[numLights].y1);
        CameraToLCD(&lights[numLights].x2, &lights[numLights].y2);
        u8g2.drawBox(lights[i].x1, lights[i].y1, lights[i].x2, lights[i].y2);
      }
    }
  } while ( u8g2.nextPage() );
  */
}