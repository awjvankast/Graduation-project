#include <Arduino.h>

#include <TinyGPS++.h>
#include <SoftwareSerial.h>

TinyGPSPlus gps;
SoftwareSerial ss(26, 27);

void setup() {
  Serial.begin(115200);
  

}

void loop() {
  // put your main code here, to run repeatedly:

}
