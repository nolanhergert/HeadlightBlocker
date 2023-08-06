#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

/* Constructor */
U8G2_ST7571_128X128_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 12, /* data=*/ 13, /* cs=*/ 1, /* dc=*/ 3, /* reset=*/ 2);

void setup() {
  // put your setup code here, to run once:
  u8g2.begin();
  u8g2.setContrast(200); // good enough for straight on. Trade off "off" transparency with "on" darkness
  // put your main code here, to run repeatedly:
  u8g2.firstPage();
  do {
    u8g2.drawBox(0,0,25,25);
  } while ( u8g2.nextPage() );
}


// TODO: Test with lower voltage? Probably can't with this...





void loop() {





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
