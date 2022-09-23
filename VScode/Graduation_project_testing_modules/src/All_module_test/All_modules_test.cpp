// TODO:
// functionality
// - implement OTA updating X
// - implement both LoRa receive code X
// - implement logging onto webserver and SD
//    - Fix data overwriting old data in SD file

// structure
//  - create multiple .cpp files with a header file to run the test code from the main code at startup
//  - look into more efficient printing than print and  println
//  -

#include <Arduino.h>

#include <SPI.h>
#include <LoRa.h>

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <SD.h>

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <AsyncElegantOTA.h>

const char *ssid = "POCO";
const char *password = "knabobar";
bool ledState  = 0;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

#define SCK 18
#define MISO 23
#define MOSI 19

#define SS_LORA 15
#define SS_ALT 0
#define SS_SD 5

#define LORA_RESET 13
#define LORA_DATA 4
#define SYNC_LORA 0xF3

#define RX_GPS 39
#define TX_GPS 27
#define GPS_NC 36
#define PPS_GPS 34
#define STANDBY_GPS 22
#define RESET_GPS 2

#define LED_WEBSERVER 32

#define GPSBAUD 9600

// The TinyGPSPlus object and initializing software serial
TinyGPSPlus gps;
SoftwareSerial ss(RX_GPS, TX_GPS);

File myFile;

void printBin(byte aByte);
void printBin16(unsigned int aByte);
String processor(const String &var);
void notifyClients(); 
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len);
void initWebSocket();

int counter = 0;
unsigned long session_identifier = 0;
unsigned long last = 0UL;

void setup()
{
  // Standard baud rate of ESP32
  Serial.begin(115200);

  // Leave all the GPS pins which are not used floating, recommended by datasheet
  pinMode(GPS_NC, INPUT);
  pinMode(PPS_GPS, INPUT);
  pinMode(STANDBY_GPS, INPUT);
  pinMode(RESET_GPS, INPUT);

  // Setting the chip/slave select pins of the SPI modules
  pinMode(SS_LORA, OUTPUT);
  digitalWrite(SS_LORA, HIGH);
  pinMode(SS_ALT, OUTPUT);
  digitalWrite(SS_ALT, HIGH);

  ss.begin(GPSBAUD);
  SPI.begin(SCK, MISO, MOSI, SS_ALT);
  SPI.setClockDivider(SPI_CLOCK_DIV64);

  pinMode(LED_WEBSERVER, OUTPUT);

  Serial.println("------------- Performing basic module test -------------");
  Serial.println("Testing LoRa, altimeter, GPS and SD functionality:");
  Serial.println("--- SPI PINS ---");
  Serial.print("    MOSI:");
  Serial.println(MOSI);
  Serial.print("    MISO:");
  Serial.println(MISO);
  Serial.print("    SCK:");
  Serial.println(SCK);
  Serial.print("    SS_LORA:");
  Serial.println(SS_LORA);
  Serial.print("    SS_ALT:");
  Serial.println(SS_ALT);
  Serial.print("    SS_SD:");
  Serial.println(SS_SD);
  Serial.println("");

  session_identifier = random(11111, 99999);
  Serial.print("Session identifier: ");
  Serial.println(session_identifier);
  Serial.println("");

  Serial.println("--- LORA INITIALIZAITON ---");
  digitalWrite(SS_LORA, LOW);
  LoRa.setPins(SS_LORA, LORA_RESET, LORA_DATA);
  while (!LoRa.begin(433E6))
  {
    Serial.print(".");
    delay(500);
  }
  LoRa.setSyncWord(SYNC_LORA);
  Serial.println("");
  Serial.println("LoRa initialized!");
  Serial.println();
  digitalWrite(SS_LORA, HIGH);

  Serial.println("--- SD INITIALIZATION ---");
  while (!SD.begin(SS_SD))
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("SD card initialized!");
  Serial.println();

  Serial.println("Writing test file to SD card");
  myFile = SD.open("/all_modules_test.txt", FILE_WRITE); // check whats happening here
  // if the file opened okay, write to it:
  if (myFile)
  {
    Serial.print("Writing to SD all_modules_test.txt...");
    myFile.print("Transceiver session ID: ");
    myFile.println(session_identifier);
    myFile.close();
    Serial.println("done");
  }
  else
  {
    // if the file didn't open, print an error:
    Serial.println("Error opening SD file");
  }

  // Creating columns for received messages
  myFile = SD.open("/ReceivedMessages.txt", FILE_WRITE);
   if (myFile)
  {
    Serial.print("Writing to SD ReceivedMessages.txt...");
    myFile.print("Session identifier");
    myFile.print(",");
    myFile.println(session_identifier);  
    
     myFile.print("Data"); myFile.print(","); myFile.println("RSSI"); 
     myFile.close();
     Serial.println("done.");
  }
  else
  {
    // if the file didn't open, print an error:
    Serial.println("Error opening SD file");
  }
  Serial.println();

  Serial.println("--- SPIFFS INITIALIZATION ---");
  while (!SPIFFS.begin(true))
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("SPIFFS initialized!");
  Serial.println();

  // Connect to Wi-Fi
  Serial.println("--- WIFI INITIALIZATION ---");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  initWebSocket();
  Serial.println("");
  Serial.println("Wi-Fi initialized!");

  // Print ESP32 Local IP Address
  Serial.print("IP adress: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Start ElegantOTA
  AsyncElegantOTA.begin(&server);
  // Start server
  server.begin();
}

unsigned long prev_time = millis();
unsigned long print_period = 2000;

void loop()
{
  // Websocket update
  ws.cleanupClients();

  // Led toggling for test purposes
  digitalWrite(LED_WEBSERVER, ledState);

  // Checking for incoming messages from LoRa module
  int packetSize = LoRa.parsePacket();
  String LoRaData;
  int LoRa_RSSI;

  // If a packet is received, continue to read the data and output it to the 
  // serial monitor and the SD card
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
  
    // Read packet
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
      Serial.print(LoRaData);
      
    // print RSSI of packet
    Serial.print("' with RSSI ");
    LoRa_RSSI = LoRa.packetRssi();
    Serial.println(LoRa_RSSI);
     }
  
   myFile = SD.open("/ReceivedMessages.txt", FILE_WRITE);
     if (myFile) {
     Serial.print("Writing to SD ReceivedMessages.txt...");
     myFile.print(LoRaData); myFile.print(","); myFile.println(LoRa_RSSI);
     myFile.close();
     Serial.println("done.");
     }
     else {
       // if the file didn't open, print an error:
       Serial.println("error opening SD file");
       delay(500);
     }
  }

  // Dispatch incoming GPS characters
  while (ss.available() > 0)
    gps.encode(ss.read());

  if (millis() - print_period > prev_time)
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
    Serial.print("Byte from reading PROM altimeter: ");
    printBin16(bite2);

    digitalWrite(SS_ALT, HIGH);
    delayMicroseconds(10);

    // Send LoRa packet to receiver
    Serial.print("Sending packet: ");
    Serial.println(counter);

    digitalWrite(SS_LORA, LOW);
    LoRa.beginPacket();
    LoRa.print("ID: ");
    LoRa.print(session_identifier);
    LoRa.print(" packet_number: ");
    LoRa.print(counter);
    LoRa.endPacket();
    counter++;
    digitalWrite(SS_LORA, HIGH);
    delayMicroseconds(100);
    prev_time = millis();

    if (gps.time.isUpdated())
    {
      Serial.print(F("TIME       Fix Age="));
      Serial.print(gps.time.age());
      Serial.print(F("ms Raw="));
      Serial.print(gps.time.value());
      Serial.print(F(" Hour="));
      Serial.print(gps.time.hour());
      Serial.print(F(" Minute="));
      Serial.print(gps.time.minute());
      Serial.print(F(" Second="));
      Serial.print(gps.time.second());
      Serial.print(F(" Hundredths="));
      Serial.println(gps.time.centisecond());
    }
  }

  AsyncElegantOTA.loop();
}

void printBin(byte aByte)
{
  for (int8_t aBit = 7; aBit >= 0; aBit--)
    Serial.print(bitRead(aByte, aBit) ? '1' : '0');
  Serial.println();
}
void printBin16(unsigned int aByte)
{
  for (int8_t aBit = 15; aBit >= 0; aBit--)
    Serial.print(bitRead(aByte, aBit) ? '1' : '0');
  Serial.println();
}

void notifyClients() {
  ws.textAll(String(ledState));
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ledState){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();

}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle") == 0) {
      ledState = !ledState;
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}