/*********
  Modified from the examples of the Arduino LoRa library
  More resources: https://randomnerdtutorials.com
*********/

#include <SPI.h>
#include <LoRa.h>

//define the pins used by the transceiver module
#define ss 10
#define rst 9
#define dio0 2

int counter = 0;
long session_identifier =0;

void setup() {
  //initialize Serial Monitor
  Serial.begin(9600);
  randomSeed(analogRead(0));
  while (!Serial);
  Serial.println("LoRa Sender");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");

  session_identifier = random(11111,99999);
  Serial.print("Session identifier: "); Serial.println(session_identifier);
}

void loop() {
  //Serial.print("Session identifier: "); Serial.println(session_identifier);
  
  Serial.print("Sending packet: "); Serial.println(counter);

  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print("ID: ");
  LoRa.print(session_identifier);
  LoRa.print(" packet_number: ");
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;

  delay(3000);
}
