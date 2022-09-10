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
const int buttonPin = 5;     // the number of the pushbutton pin
const int  led_Pin = 4;     // the number of the pushbutton pin

#define CALIB_SEC 20
#define SEND_INTERVAL 100


int counter = 0;
long session_identifier =0;
int buttonState = 0;

void setup() {
  //initialize Serial Monitor
  Serial.begin(9600);
  pinMode(led_Pin,OUTPUT);
  pinMode(buttonPin, INPUT);
  randomSeed(analogRead(0));
  while (!Serial);
  Serial.println("LoRa Sender");

  Wire.begin(); // needed for IMU mag calibration
  mySensor.setWire(&Wire);
  
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
  Serial.println(F("LoRa Initializing OK!"));

  session_identifier = random(11111,99999);
  Serial.print(F("Session identifier: ")); Serial.println(session_identifier);

  // Calibration of mag sensor
   mySensor.beginMag();

  float magXMin, magXMax, magYMin, magYMax, magZ, magZMin, magZMax;

  digitalWrite(4,HIGH);
  Serial.println(F("Start scanning values of magnetometer to get offset values."));
  Serial.println("Rotate your device for " + String(CALIB_SEC) + " seconds.");
  setMagMinMaxAndSetOffset(&mySensor, CALIB_SEC);
  Serial.println("Finished setting offset values.");

   mySensor.beginAccel();
  mySensor.beginGyro();

  digitalWrite(4,LOW); 
  Serial.println("mySensor.magXOffset = " + String(mySensor.magXOffset) + ";");
  Serial.println("mySensor.maxYOffset = " + String(mySensor.magYOffset) + ";");
  Serial.println("mySensor.magZOffset = " + String(mySensor.magZOffset) + ";");
}

void loop() {
    uint8_t sensorId;
  buttonState = digitalRead(buttonPin);
  if(buttonState == HIGH){
    digitalWrite(led_Pin,HIGH);
     session_identifier = random(11111,99999);
     Serial.println("");
     Serial.println(F("-------------------------"));
     Serial.print(F("Session identifier: ")); Serial.println(session_identifier);
     counter = 0;
      while(buttonState == HIGH){
        buttonState = digitalRead(buttonPin);
      }
      digitalWrite(led_Pin,LOW);
  }
    
  if (mySensor.readId(&sensorId) == 0) {
    Serial.println("sensorId: " + String(sensorId));
  } else {
    Serial.println(F("Cannot read sensorId"));
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
    Serial.println(F("Cannot read gyro values"));
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
    Serial.println(F("Cannot read mag values"));
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

  delay(SEND_INTERVAL);
}

void setMagMinMaxAndSetOffset(MPU9250_asukiaaa* sensor, int seconds) {
  unsigned long calibStartAt = millis();
  float magX, magXMin, magXMax, magY, magYMin, magYMax, magZ, magZMin, magZMax;

  sensor->magUpdate();
  magXMin = magXMax = sensor->magX();
  magYMin = magYMax = sensor->magY();
  magZMin = magZMax = sensor->magZ();

  while(millis() - calibStartAt < (unsigned long) seconds * 1000) {
    delay(100);
    sensor->magUpdate();
    magX = sensor->magX();
    magY = sensor->magY();
    magZ = sensor->magZ();
    if (magX > magXMax) magXMax = magX;
    if (magY > magYMax) magYMax = magY;
    if (magZ > magZMax) magZMax = magZ;
    if (magX < magXMin) magXMin = magX;
    if (magY < magYMin) magYMin = magY;
    if (magZ < magZMin) magZMin = magZ;
  }

  sensor->magXOffset = - (magXMax + magXMin) / 2;
  sensor->magYOffset = - (magYMax + magYMin) / 2;
  sensor->magZOffset = - (magZMax + magZMin) / 2;
}
