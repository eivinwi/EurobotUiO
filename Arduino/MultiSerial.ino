#include <Arduino.h>



void setup() {
  // initialize both serial ports:
  Serial.begin(38400);
  Serial1.begin(38400);
  delay(100);
/*  for(int i = 0; i < 100; i++) {
    Serial1.write((byte) 0x00);
    Serial1.write(0x26);
    delay(50);
  }*/
}


void loop() {
  // read from port 1, send to port 0:
  if (Serial1.available()) {
    int inByte = Serial1.read();
    Serial.write(inByte);
  }
 
  // read from port 0, send to port 1:
  if (Serial.available()) {
    int inByte = Serial.read();
    Serial1.write(inByte);
  }


}