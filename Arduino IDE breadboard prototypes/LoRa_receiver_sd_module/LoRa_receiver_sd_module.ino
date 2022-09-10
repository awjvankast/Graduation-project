/*********
  Modified from the examples of the Arduino LoRa library
  More resources: https://randomnerdtutorials.com
*********/

#include <SPI.h>
#include <LoRa.h>

#include <SD.h>
File myFile;

//define the pins used by the transceiver module
#define ss 10
#define rst 9
#define dio0 2
#define CHIP_SELECT_SD 3

long session_identifier = 0;

void setup() {
  //initialize Serial Monitor
  Serial.begin(9600);
  randomSeed(analogRead(0));
  session_identifier = random(11111,99999);
  
  Serial.print("Receiver session ID:"); Serial.println(session_identifier);
  while (!Serial);
  Serial.println("LoRa Receiver");

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
  
  Serial.print("Initializing SD card...");
  if (!SD.begin(CHIP_SELECT_SD)) {
  Serial.println("initialization failed!");
  while (1);
  }
  Serial.println("initialization done.");
  myFile = SD.open("rb.txt", FILE_WRITE); // check whats happening here
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to SD...");
    myFile.print("Receiver session ID: ");
    myFile.println(session_identifier);
    myFile.close();
    Serial.println("done.");
  } else {
  // if the file didn't open, print an error:
  Serial.println("error opening SD file");
  }
}  


void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();

  String LoRaData;
  int LoRa_RSSI;
  
  
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
  
    // read packet
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
      Serial.print(LoRaData);
      
    // print RSSI of packet
    Serial.print("' with RSSI ");
    LoRa_RSSI = LoRa.packetRssi();
    Serial.println(LoRa_RSSI);
     }
  
   myFile = SD.open("rb.txt", FILE_WRITE);
     if (myFile) {
     Serial.print("Writing to SD...");
     myFile.print("Data: ");
     myFile.print(LoRaData);
     myFile.print(" with RSSI: ");
     myFile.println(LoRa_RSSI);
     myFile.close();
     Serial.println("done.");
     }
     else {
       // if the file didn't open, print an error:
       Serial.println("error opening SD file");
       delay(500);
     }

  }
  
}
