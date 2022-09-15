#include <Arduino.h>

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

static const int RXPin = 39, TXPin = 27;
static const uint32_t GPSBaud = 9600;

// The TinyGPSPlus object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

void setup()
{
  Serial.begin(9600);
    pinMode(36,INPUT);
  pinMode(34,INPUT);
  pinMode(22,INPUT);
  pinMode(2,INPUT); /// THIS WORKS
  ss.begin(GPSBaud);

  Serial.println("GPS initialized");

}

void loop()
{
 while (ss.available())
  {
    char c = ss.read();  // <--- get the incoming character
    Serial.print (c);     // <--- print it
    if (gps.encode(c))    // <--- now pass it to TinyGPS
      return;
  }
}