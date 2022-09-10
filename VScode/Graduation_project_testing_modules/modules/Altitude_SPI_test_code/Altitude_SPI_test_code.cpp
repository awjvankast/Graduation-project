#include <Arduino.h>

// wrong pin assignments done on purpose, test if they can be changed in the loop
#define MISO 19
#define MOSI 23
#define SS 25

#include <SPI.h>

void printBin(byte aByte);
void printBin16(unsigned int aByte);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("CHECK");
  pinMode(SS, OUTPUT);
  SPI.begin(SCK, MISO, MOSI, SS);
  SPI.setClockDivider(SPI_CLOCK_DIV64);
  //SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));


  digitalWrite(SS, HIGH);


}

void loop() {
  // put your main code here, to run repeatedly:
    Serial.print("MOSI:");  Serial.println(MOSI);
  Serial.print("MISO:");  Serial.println(MISO);
  Serial.print("SCK:");  Serial.println(SCK);
  Serial.print("SS:");  Serial.println(SS);


  digitalWrite(SS, LOW);
  SPI.transfer(0x1E); //reset
  delay(3);
  digitalWrite(SS, HIGH);
  delayMicroseconds(100);
  digitalWrite(SS, LOW);
  delayMicroseconds(10);
  SPI.transfer(0xA4); //sending 8 bit command
  delayMicroseconds(20);
  unsigned int bite1 = SPI.transfer16(0x0000); // sending 0

  printBin16(bite1);

  digitalWrite(SS, HIGH);
  delay(10);
  Serial.println();

  // and store read data into three bytes
  /* Serial.println("Printing bytes");
    byte byte1 = SPI.transfer(0xA1);
    delay(10);
    byte  byte2 = SPI.transfer(0xA1);
    delay(10);
    byte byte3 = SPI.transfer(0xA1);
    delay(10);
    byte byte4 = SPI.transfer(0xA1);
    delay(10);



    printBin(byte1);
    printBin(byte2);
    printBin(byte3);
     printBin(byte4);
    Serial.println();
    Serial.println("Unsigned");
    unsigned int ubyte1 = SPI.transfer16(0xA1);
    delay(10);
    unsigned int  ubyte2 = SPI.transfer16(0xA1);
    delay(10);
    unsigned int ubyte3 = SPI.transfer16(0xA1);
    delay(10);
    unsigned int ubyte4 = SPI.transfer16(0xA1);
    delay(10);

    digitalWrite(SS,HIGH);

    Serial.println(ubyte1);
    Serial.println(ubyte2);
    Serial.println(ubyte3);
     Serial.println(ubyte4);
    Serial.println("bin format");
    Serial.println(ubyte1,BIN);
    Serial.println(ubyte2,BIN);
    Serial.println(ubyte3,BIN);
     Serial.println(ubyte4,BIN);

     Serial.println();
       Serial.println();
    Serial.println();
    //digitalWrite(SS,HIGH);*/
  
   delay(500);
    SPI.end();
    delay(1000);
    // this work but be carefull only to do it at the beginning, at the end of the loop the #defines will go back to their inital global values instead of these local values
    #define SCK 18 
    #define MISO 23
    #define MOSI 19
    #define SS 0
    delay(100); 
  pinMode(SS, OUTPUT);
  digitalWrite(SS,HIGH);
    Serial.print("chip select:");Serial.println(SS);
     SPI.begin(SCK, MISO, MOSI, SS);
    
      SPI.setClockDivider(SPI_CLOCK_DIV64);

      
  digitalWrite(SS, LOW);
  SPI.transfer(0x1E); //reset
  delay(3);
  digitalWrite(SS, HIGH);
  delayMicroseconds(100);
  digitalWrite(SS, LOW);
  delayMicroseconds(10);
  SPI.transfer(0xA4); //sending 8 bit command
  delayMicroseconds(20);
  unsigned int bite2 = SPI.transfer16(0x0000); // sending 0
 Serial.print("Byte after the defines have been set in the loop: ");
  printBin16(bite2);

  digitalWrite(SS, HIGH);
  delay(10);
  Serial.println();

  
  delay(2000);



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
