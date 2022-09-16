#include <Arduino.h>

#include <SPI.h>
#include <LoRa.h>

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

#define SCK 18
#define MISO 23
#define MOSI 19

#define SS_LORA 15
#define SS_ALT 0

#define LORA_RESET 13
#define LORA_DATA 4
#define SYNC_LORA 0xF3

#define RX_GPS 39
#define TX_GPS 27
#define GPS_NC 36
#define PPS_GPS 34
#define STANDBY_GPS 22
#define RESET_GPS 2

#define GPSBAUD 9600

// The TinyGPSPlus object and initializing software serial
TinyGPSPlus gps;
SoftwareSerial ss(RX_GPS, TX_GPS);

void printBin(byte aByte);
void printBin16(unsigned int aByte);

int counter = 0;
unsigned long session_identifier = 0;
unsigned long last = 0UL;

void setup()
{
  // Standard baud rate of ESP32
  Serial.begin(115200);

  // Leave all the GPS pins which are not used floating, recommended by datasheet
  pinMode(GPS_NC, INPUT);
  pinMode(PPS_GPS, INPUT);
  pinMode(STANDBY_GPS, INPUT);
  pinMode(RESET_GPS, INPUT);

  pinMode(SS_LORA, OUTPUT);
  digitalWrite(SS_LORA, HIGH);
  pinMode(SS_ALT, OUTPUT);
  digitalWrite(SS_ALT, HIGH);

  ss.begin(GPSBAUD);
  SPI.begin(SCK, MISO, MOSI, SS_ALT);
  SPI.setClockDivider(SPI_CLOCK_DIV64);

  Serial.println("------------- Performing basic module test -------------");
  Serial.println("Testing LoRa, altimeter, GPS and SD functionality:");
  Serial.println("--- SPI PINS ---");
  Serial.print("    MOSI:");
  Serial.println(MOSI);
  Serial.print("    MISO:");
  Serial.println(MISO);
  Serial.print("    SCK:");
  Serial.println(SCK);
  Serial.print("    SS_LORA:");
  Serial.println(SS_LORA);
  Serial.print("    SS_ALT:");
  Serial.println(SS_ALT);
//  Serial.print("SS_SD:");
//  Serial.println(SS_SD);
  Serial.println("");

  digitalWrite(SS_LORA, LOW);
  LoRa.setPins(SS_LORA, LORA_RESET, LORA_DATA);

  Serial.println("Starting LoRa initialization");
  while (!LoRa.begin(433E6))
  {
    Serial.print(".");
    delay(500);
  }
  LoRa.setSyncWord(SYNC_LORA);
  Serial.println(""); Serial.println("LoRa initialized!");
  
  session_identifier = random(11111, 99999);
  Serial.print("Session identifier: ");
  Serial.println(session_identifier);
  digitalWrite(SS_LORA,HIGH);

  Serial.println("");
}

void loop()
{
  // Dispatch incoming GPS characters
  while (ss.available() > 0)
    gps.encode(ss.read());

  // Check a value of the altimeter
  digitalWrite(SS_ALT, LOW);
  SPI.transfer(0x1E); // reset
  delay(3);
  digitalWrite(SS_ALT, HIGH);
  delayMicroseconds(100);
  digitalWrite(SS_ALT, LOW);
  delayMicroseconds(10);
  SPI.transfer(0xA4); // sending 8 bit command
  delayMicroseconds(20);
  unsigned int bite2 = SPI.transfer16(0x0000); // sending 0
  Serial.print("Byte after the defines have been set in the loop: ");
  printBin16(bite2);

  digitalWrite(SS_ALT, HIGH);
  delayMicroseconds(10);

  // Send LoRa packet to receiver
  Serial.print("Sending packet: ");
  Serial.println(counter);

  digitalWrite(SS_LORA,LOW);
  LoRa.beginPacket();
  LoRa.print("ID: ");
  LoRa.print(session_identifier);
  LoRa.print(" packet_number: ");
  LoRa.print(counter);
  LoRa.endPacket();
  counter++;
  digitalWrite(SS_LORA,HIGH);
  delayMicroseconds(100);

  if (gps.time.isUpdated())
  {
    Serial.print(F("TIME       Fix Age="));
    Serial.print(gps.time.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.time.value());
    Serial.print(F(" Hour="));
    Serial.print(gps.time.hour());
    Serial.print(F(" Minute="));
    Serial.print(gps.time.minute());
    Serial.print(F(" Second="));
    Serial.print(gps.time.second());
    Serial.print(F(" Hundredths="));
    Serial.println(gps.time.centisecond());
  }
  
  
}

void printBin(byte aByte) {
  for (int8_t aBit = 7; aBit >= 0; aBit--)
    Serial.print(bitRead(aByte, aBit) ? '1' : '0');
  Serial.println();
}
void printBin16(unsigned int aByte) {
  for (int8_t aBit = 15; aBit >= 0; aBit--)
    Serial.print(bitRead(aByte, aBit) ? '1' : '0');
  Serial.println();
}
