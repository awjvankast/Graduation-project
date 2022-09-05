#include <SPI.h>
#include <LoRa.h>

#define SS 15

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
}

void loop() {
  // put your main code here, to run repeatedly:

}
