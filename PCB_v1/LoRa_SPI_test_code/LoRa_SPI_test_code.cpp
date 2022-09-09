#include <Arduino.h>

#include <SPI.h>
#include <LoRa.h>

#define MISO 23
#define MOSI 19

#define SS 15
#define LORA_RESET 13
#define LORA_DATA 4

int counter = 0;
long session_identifier =0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(SS, OUTPUT);
  digitalWrite(SS, HIGH);
  SPI.begin(SCK, MISO, MOSI, SS);
  SPI.setClockDivider(SPI_CLOCK_DIV64);

  Serial.print("MOSI:");  Serial.println(MOSI);
  Serial.print("MISO:");  Serial.println(MISO);
  Serial.print("SCK:");  Serial.println(SCK);
  Serial.print("SS:");  Serial.println(SS);

    digitalWrite(SS, LOW);
  LoRa.setPins(SS, LORA_RESET, LORA_DATA);

  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }
   LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
   
   session_identifier = random(11111,99999);
  Serial.print("Session identifier: "); Serial.println(session_identifier);
}

void loop() {
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
