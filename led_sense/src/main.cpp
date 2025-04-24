
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

const int LED_COVERED_SENSE_LOW = A5; // "Ground" of normal forward bias LED
const int LED_COVERED_SENSE_HIGH = A7; // VCC side of normal forward bias LED

const int LED_UNCOVERED_SENSE_HIGH = A3;
const int LED_UNCOVERED_SENSE_LOW = A2;

#define ADC_REVERSE_BIAS_SENSE_VOLTAGE
//#define ADC_REVERSE_BIAS_SENSE_CURRENT

// the setup function runs once when you press reset or power the board
void setup() {

  Serial.begin(115200);

}

uint32_t startMicros = 0;
uint32_t sum = 0;
uint32_t reps = 1;
char str[50] = "";
volatile uint32_t led_covered_val = 0;
volatile uint32_t led_uncovered_val = 0;

void loop() {
  
  /*
  // Forward biased directed into ADC pin, sense with ADC
  // Very noisy still
  pinMode(LED_COVERED_SENSE_HIGH, INPUT);
  digitalWrite(LED_COVERED_SENSE_HIGH, LOW);
  pinMode(LED_COVERED_SENSE_LOW, INPUT);
  //Serial.print("x: ");
  sum = 0;
  for (int i = 0; i < reps; i++) {
    sum += analogRead(LED_COVERED_SENSE_LOW);
  }
  Serial.println(sum / reps);
  */
  

#ifdef ADC_REVERSE_BIAS_SENSE_VOLTAGE
  // Reverse bias, adc is sensing high side voltage decay
  pinMode(LED_COVERED_SENSE_HIGH, OUTPUT);
  digitalWrite(LED_COVERED_SENSE_HIGH, LOW);
  pinMode(LED_COVERED_SENSE_LOW, OUTPUT);
  digitalWrite(LED_COVERED_SENSE_LOW, HIGH); // Charge
  pinMode(LED_UNCOVERED_SENSE_HIGH, OUTPUT);
  digitalWrite(LED_UNCOVERED_SENSE_HIGH, LOW);
  pinMode(LED_UNCOVERED_SENSE_LOW, OUTPUT);
  digitalWrite(LED_UNCOVERED_SENSE_LOW, HIGH); // Charge
  delayMicroseconds(10);
  pinMode(LED_COVERED_SENSE_LOW, INPUT);
  pinMode(LED_UNCOVERED_SENSE_LOW, INPUT);
  delay(5);
  led_uncovered_val = analogRead(LED_UNCOVERED_SENSE_LOW);
  led_uncovered_val = analogRead(LED_UNCOVERED_SENSE_LOW);
  led_uncovered_val = analogRead(LED_UNCOVERED_SENSE_LOW);
  led_uncovered_val = analogRead(LED_UNCOVERED_SENSE_LOW);
  delayMicroseconds(100);
  led_covered_val = analogRead(LED_COVERED_SENSE_LOW);
  led_covered_val = analogRead(LED_COVERED_SENSE_LOW);
  led_covered_val = analogRead(LED_COVERED_SENSE_LOW);
  led_covered_val = analogRead(LED_COVERED_SENSE_LOW);
  led_covered_val = analogRead(LED_COVERED_SENSE_LOW);
  //sprintf(str, "%lu - %lu = %ld", led_uncovered_val, led_covered_val, led_uncovered_val - led_covered_val);
  sprintf(str, "a: %lu b: %ld",led_uncovered_val, led_covered_val);
  Serial.println(str);
#endif



  // Maybe try this along with another LED and subtract values for a basic differential amplifier?
#ifdef ADC_REVERSE_BIAS_SENSE_CURRENT
  // Reverse bias, but "output" current of LED (bottom) is charging adc capacitor, hopefully quickly
  // Although it does pick up the minimum amount of light, the response time is *really* slow, ~1 second, can't figure out why
  // It seems to be with the LED and not the ADC, as pre-reading the ADC a bunch doesn't change it
  // Although if I don't pre-read, the readings are quite high, around 900. So maybe atmega328 doesn't clear the adc before reading?
  pinMode(LED_COVERED_SENSE_LOW, OUTPUT);
  digitalWrite(LED_COVERED_SENSE_LOW, LOW);
  // Optional, doesn't seem to change slow response
  //pinMode(LED_COVERED_SENSE_LOW, INPUT);
  pinMode(LED_COVERED_SENSE_HIGH, INPUT);
  delay(10);
  sum = 0;
  for (int i = 0; i < reps; i++) {
    sum += analogRead(LED_COVERED_SENSE_HIGH);
    delay(1);
  }
  Serial.println(sum / reps);//analogRead(LED_COVERED_SENSE_HIGH));

  // Briefly clear embedded charges using forward bias for a bit
  pinMode(LED_COVERED_SENSE_HIGH, OUTPUT);
  digitalWrite(LED_COVERED_SENSE_HIGH, HIGH);
  pinMode(LED_COVERED_SENSE_LOW, OUTPUT);
  digitalWrite(LED_COVERED_SENSE_LOW, LOW);
  delayMicroseconds(100);
  // Clear this again?
  digitalWrite(LED_COVERED_SENSE_HIGH, LOW);
  digitalWrite(LED_COVERED_SENSE_LOW, HIGH);
  delay(1);
  
#endif  
  

  /*  
  // Charge LED, reverse-biased
  pinMode(LED_COVERED_SENSE_HIGH, OUTPUT);
  digitalWrite(LED_COVERED_SENSE_HIGH, LOW);
  pinMode(LED_COVERED_SENSE_LOW, OUTPUT);
  digitalWrite(LED_COVERED_SENSE_LOW, HIGH); // Charge 
  delay(1);
  pinMode(LED_COVERED_SENSE_LOW, INPUT);
  startMicros = micros();
  // Wait for LED capacitance to decay
  while (digitalRead(LED_COVERED_SENSE_LOW) == 1) {
    delayMicroseconds(1); // How frequently this is read surprisingly doesn't matter! (reading more often doesn't decay it quicker)
  }
  Serial.println((micros() - startMicros));
  */

  


  /*
  // Blink LED once
  pinMode(LED_COVERED_SENSE_HIGH, OUTPUT);
  digitalWrite(LED_COVERED_SENSE_HIGH, HIGH);
  pinMode(LED_COVERED_SENSE_LOW, OUTPUT);
  digitalWrite(LED_COVERED_SENSE_LOW, LOW);
  delay(1);


  pinMode(LED_COVERED_SENSE_LOW, INPUT_PULLUP);
  pinMode(LED_COVERED_SENSE_HIGH, INPUT_PULLUP);
  delay(250);
  

  pinMode(LED_UNCOVERED_SENSE_HIGH, OUTPUT);
  digitalWrite(LED_UNCOVERED_SENSE_HIGH, HIGH);
  pinMode(LED_UNCOVERED_SENSE_LOW, OUTPUT);
  digitalWrite(LED_UNCOVERED_SENSE_LOW, LOW);
  delay(1);


  pinMode(LED_UNCOVERED_SENSE_LOW, INPUT_PULLUP);
  pinMode(LED_UNCOVERED_SENSE_HIGH, INPUT_PULLUP);
  delay(250);
  
  
  */

}
