
#include <Arduino.h>
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

  http://www.arduino.cc/en/Tutorial/Blink
*/

const int LED_SENSE_LOW = D6; // "Ground" of normal forward bias LED
const int LED_SENSE_HIGH = D7; // VCC side of normal forward bias LED

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_SENSE_LOW, OUTPUT);
  pinMode(LED_SENSE_HIGH, OUTPUT);
  Serial.begin(115200);

}

// the loop function runs over and over again forever
uint32_t startMicros = 0;
void loop() {

  Serial.println(analogRead(A0));
  delay(10);
/*
  // Charge LED, reverse-biased
  pinMode(LED_SENSE_HIGH, OUTPUT);
  digitalWrite(LED_SENSE_HIGH, LOW);
  pinMode(LED_SENSE_LOW, OUTPUT);
  digitalWrite(LED_SENSE_LOW, HIGH); // Charge with pull up resistor
  delay(1);
  pinMode(LED_SENSE_LOW, INPUT);
  startMicros = micros();
  for (int i = 0; i < 100; i++) {
    Serial.print(digitalRead(LED_SENSE_LOW));
    delayMicroseconds(100);
  }

  //while (digitalRead(LED_SENSE_LOW) == 1) {};
  //Serial.printf("Time elapsed: %d us\n", micros() - startMicros);

  // Test that the LED is plugged in the right way
  delay(250);
  pinMode(LED_SENSE_HIGH, INPUT_PULLUP);
  //digitalWrite(LED_SENSE_HIGH, );
  pinMode(LED_SENSE_LOW, OUTPUT);
  digitalWrite(LED_SENSE_LOW, LOW);
  delay(100);
  */

}
