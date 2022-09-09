#include <SD.h>
#include <SPI.h>

File myFile;

#define SS_SD 5
#define SS 5


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(SS_SD, OUTPUT);
  digitalWrite(SS_SD, HIGH);
  SPI.begin(SCK, MISO, MOSI, SS_SD);
  SPI.setClockDivider(SPI_CLOCK_DIV64);

  Serial.print("MOSI:");  Serial.println(MOSI);
  Serial.print("MISO:");  Serial.println(MISO);
  Serial.print("SCK:");  Serial.println(SCK);
  Serial.print("SS:");  Serial.println(SS);

  digitalWrite(SS_SD, LOW);

  Serial.print("Initializing SD card...");
  if (!SD.begin(SS_SD)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  myFile = SD.open("rb.txt", FILE_WRITE); // check whats happening here
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to SD...");
    myFile.print("Receiver session ID: CHECK");

    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening SD file");
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}
