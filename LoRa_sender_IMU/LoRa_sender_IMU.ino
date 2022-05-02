/*********
  Modified from the examples of the Arduino LoRa library
  More resources: https://randomnerdtutorials.com
*********/

#include <SPI.h>
#include <LoRa.h>

// MPU initialization 
#include <MPU9250_asukiaaa.h>
MPU9250_asukiaaa mySensor;
float aX, aY, aZ, aSqrt, gX, gY, gZ, mDirection, mX, mY, mZ;

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

  // starting MPU sensors
  mySensor.beginAccel();
  mySensor.beginGyro();
  mySensor.beginMag();

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
    uint8_t sensorId;
  if (mySensor.readId(&sensorId) == 0) {
    Serial.println("sensorId: " + String(sensorId));
  } else {
    Serial.println("Cannot read sensorId");
  }

  if (mySensor.accelUpdate() == 0) {
    aX = mySensor.accelX();
    aY = mySensor.accelY();
    aZ = mySensor.accelZ();
    aSqrt = mySensor.accelSqrt();
    Serial.println("accelX: " + String(aX));
    Serial.println("accelY: " + String(aY));
    Serial.println("accelZ: " + String(aZ));
    Serial.println("accelSqrt: " + String(aSqrt));
  } else {
    Serial.println("Cannod read accel values");
  }

  if (mySensor.gyroUpdate() == 0) {
    gX = mySensor.gyroX();
    gY = mySensor.gyroY();
    gZ = mySensor.gyroZ();
    Serial.println("gyroX: " + String(gX));
    Serial.println("gyroY: " + String(gY));
    Serial.println("gyroZ: " + String(gZ));
  } else {
    Serial.println("Cannot read gyro values");
  }

  if (mySensor.magUpdate() == 0) {
    mX = mySensor.magX();
    mY = mySensor.magY();
    mZ = mySensor.magZ();
    mDirection = mySensor.magHorizDirection();
    Serial.println("magX: " + String(mX));
    Serial.println("maxY: " + String(mY));
    Serial.println("magZ: " + String(mZ));
    Serial.println("horizontal direction: " + String(mDirection));
  } else {
    Serial.println("Cannot read mag values");
  }

  Serial.println("at " + String(millis()) + "ms");
  Serial.println(""); // Add an empty line

  
  Serial.print("Sending packet: "); Serial.println(counter);

  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print("ID:");
  LoRa.print(session_identifier);
  LoRa.print(",p_no:");
  LoRa.print(counter);
  LoRa.print(",accel:");
  LoRa.print(aX); LoRa.print(","); LoRa.print(aY); LoRa.print(","); LoRa.print(aZ);
  LoRa.print(",gyro:");
  LoRa.print(gX); LoRa.print(","); LoRa.print(gY); LoRa.print(","); LoRa.print(gZ);
  LoRa.print(",mag:");
  LoRa.print(mX); LoRa.print(","); LoRa.print(mY); LoRa.print(","); LoRa.print(mZ);
  LoRa.endPacket();

  counter++;

  delay(1000);
}
