#include <Arduino.h>

#include <SPI.h>
#include <LoRa.h>

#define SCK 18
#define MISO 23
#define MOSI 19

#define SS_LORA 15
#define SS_ALT 0

#define LORA_RESET 13
#define LORA_DATA 4

void printBin(byte aByte);
void printBin16(unsigned int aByte);

int counter = 0;
long session_identifier = 0;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(SS_LORA, OUTPUT);
  digitalWrite(SS_LORA, HIGH);
  pinMode(SS_ALT, OUTPUT);
  digitalWrite(SS_ALT, HIGH);

  SPI.begin(SCK, MISO, MOSI, SS_ALT);
  SPI.setClockDivider(SPI_CLOCK_DIV64);

  Serial.print("MOSI:");
  Serial.println(MOSI);
  Serial.print("MISO:");
  Serial.println(MISO);
  Serial.print("SCK:");
  Serial.println(SCK);
  Serial.print("SS_LORA:");
  Serial.println(SS_LORA);
  Serial.print("SS_ALT:");
  Serial.println(SS_ALT);

  digitalWrite(SS_LORA, LOW);
  LoRa.setPins(SS_LORA, LORA_RESET, LORA_DATA);
  
  while (!LoRa.begin(433E6))
  {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
  
  session_identifier = random(11111, 99999);
  Serial.print("Session identifier: ");
  Serial.println(session_identifier);

  digitalWrite(SS_LORA,HIGH);
}

void loop()
{
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
  delay(10);

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
  
  delay(3000);
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
