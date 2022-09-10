/*********
  Modified from the examples of the Arduino LoRa library
  More resources: https://randomnerdtutorials.com
*********/

#include <SPI.h>
#include <LoRa.h>

#include <SD.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//define the pins used by the transceiver module
#define ss 10
#define rst 9
#define dio0 2

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

    display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Initializing display...");
  display.display(); 
  
  while (!Serial);
  Serial.println("LoRa Receiver");
  display.println("LoRa Receiver");
  display.display(); 

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    display.println(".");
    display.display(); 
    delay(500);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
  display.println("LoRa Initializing OK!");
  display.display(); 
  delay(500);
  display.clearDisplay();
  display.setCursor(0, 10);
}

int line_count = 0;
void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    if(line_count == 6){
      line_count = 0;
      display.clearDisplay();
      display.setCursor(0, 10);
    }
    // received a packet
    Serial.print("Rp:");
    display.print("Rp:");
    display.display(); 

    // read packet
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData); 
      display.print(LoRaData);
      display.display(); 
    }

    // print RSSI of packet
    Serial.print(",RS:");
    display.print(",RS:");
    display.display(); 
    Serial.println(LoRa.packetRssi());
    display.println(LoRa.packetRssi());
    display.display(); 

    line_count++;
  }
  
}
