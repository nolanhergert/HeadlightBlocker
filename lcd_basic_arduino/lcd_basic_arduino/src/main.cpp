

/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/
#include <Arduino.h>

const int LCD_BACKPLANE = 5;
const int LCD_SEGMENT = 6;
const int LCD_DELAY_MS_MAX = 1000/30/2; // 30FPS, switched twice
const int LCD_CHARGE_TIME_MS = 1;
const int CAMERA_DELAY_US = 1; // 30FPS
const int CAMERA_SWITCH_ITERATIONS = 500; //1ms is ok still
int16_t lcd_duty_cycle_diff = 1;
uint16_t lcd_duty_cycle = lcd_duty_cycle_diff+1;
uint16_t lcd_duty_cycle_min = 0;
// Driving with 5V is causing ghosting, so it's a 3.3V LCD.
// Doesn't seem to be frequency dependent, but is temperature dependent!
// Still getting ghosting even at really low duty cycle (>20%)! So...need
// to think about this some more, maybe lower 5V down to 3.3V so you can raw pwm still
uint16_t lcd_duty_cycle_max = 40;



// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(LCD_BACKPLANE, OUTPUT);
  pinMode(LCD_SEGMENT, OUTPUT);
  Serial.begin(115200);
}

// the loop function runs over and over again forever
void loop() {

  // Basic setup
  digitalWrite(LCD_BACKPLANE, LOW);
  digitalWrite(LCD_SEGMENT, HIGH);
  delay(LCD_DELAY_MS_MAX);

  digitalWrite(LCD_BACKPLANE, HIGH);
  digitalWrite(LCD_SEGMENT, LOW);
  delay(LCD_DELAY_MS_MAX);


  /*
  // See if I can get a "draining" effect to neighboring pixels. I'm sure it happens (the lcd segments dim somewhat quickly), but not enough to turn the others on
  // Charge up segments quickly
  pinMode(LCD_BACKPLANE, OUTPUT);
  digitalWrite(LCD_BACKPLANE, LOW);
  digitalWrite(LCD_SEGMENT, HIGH);
  delay(LCD_CHARGE_TIME_MS);
  // Now set backplane to high-z and see what happens for draining
  pinMode(LCD_BACKPLANE, INPUT);
  delay(1000/5);

  // Now do same thing, just reverse polarity
  pinMode(LCD_BACKPLANE, OUTPUT);
  digitalWrite(LCD_BACKPLANE, HIGH);
  digitalWrite(LCD_SEGMENT, LOW);
  delay(LCD_CHARGE_TIME_MS);
  // Now set backplane to high-z and see what happens for draining
  pinMode(LCD_BACKPLANE, INPUT);
  delay(1000/5);
  */


/*
  // Fading in/out, works well, I think!
  digitalWrite(LCD_BACKPLANE, LOW);
  analogWrite(LCD_SEGMENT, lcd_duty_cycle);
  delay(LCD_DELAY_MS_MAX);
  digitalWrite(LCD_BACKPLANE, HIGH);
  // Write inverted to add up to 0 DC
  analogWrite(LCD_SEGMENT, lcd_duty_cycle_max-lcd_duty_cycle);
  delay(LCD_DELAY_MS_MAX);
  lcd_duty_cycle += lcd_duty_cycle_diff;
  if (lcd_duty_cycle > (lcd_duty_cycle_max - abs(lcd_duty_cycle_diff)) || lcd_duty_cycle < (lcd_duty_cycle_min + abs(lcd_duty_cycle_diff))) {
    lcd_duty_cycle_diff *= -1;
  }
  //Serial.println(lcd_duty_cycle);
*/

/*

  // Simulate sharing the pin with a camera. Works well when fast enough. Not sure why it drains though
  pinMode(LCD_BACKPLANE, INPUT);
  pinMode(LCD_SEGMENT, OUTPUT);
  for (int i = 0; i < CAMERA_SWITCH_ITERATIONS; i++) {
    digitalWrite(LCD_SEGMENT, LOW);
    delayMicroseconds(CAMERA_DELAY_US);
    digitalWrite(LCD_SEGMENT, HIGH);
    delayMicroseconds(CAMERA_DELAY_US);
  }
  pinMode(LCD_BACKPLANE, OUTPUT);
  */

}
