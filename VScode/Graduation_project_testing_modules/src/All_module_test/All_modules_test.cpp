// TODO:
// functionality
// - implement OTA updating X
// - implement both LoRa receive code X
// - implement logging onto webserver and SD X
//    - Fix data overwriting old data in SD file X
// - Add timestamps to data logs, see tutorial in notes X
// - Make periodic SD card presence check and log it to webpage 
// - Fix cables and plugs of battery pack and usb connector
// - Get more SD cards

// structure
//  - create multiple .cpp files with a header file to run the test code from the main code at startup
//  - look into more efficient printing than print and  println --> printf can use arguments with %
//  -

#include <Arduino.h>

#include <LoRa.h>

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <AsyncElegantOTA.h>

// CHANGE THIS for every different node
String NodeName = "Famke";
int last_IP_number = 6;

const char *ssid = "POCO";
const char *password = "knabobar";
bool ledState = 0;
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

#define BAT_SENSE 35

#define LED_LOW_BAT 33
#define LED_WEBSERVER 32

#define GPSBAUD 9600

// The TinyGPSPlus object and initializing software serial
TinyGPSPlus gps;
SoftwareSerial ss(RX_GPS, TX_GPS);

File myFile;

// Set your Static IP address
IPAddress local_IP(192, 168, 43, last_IP_number); // A = 1, B = 2, ...
// Set your Gateway IP address
IPAddress gateway(192, 168, 43, last_IP_number);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   // optional
IPAddress secondaryDNS(8, 8, 4, 4); // optional

void printBin(byte aByte);
void printBin16(unsigned int aByte);
String processor(const String &var);
void notifyClients();
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len);
void initWebSocket();
void initSDCard();
void writeFile(fs::FS &fs, const char *path, const char *message);
void appendFile(fs::FS &fs, const char *path, const char *message);
void printLocalTime();

void updateHTML_LoRa(String LoRaMessage);

int counter = 0;
unsigned long session_identifier = 0;
unsigned long last = 0UL;
char buffer[50];
int SD_present = 0;

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

  // Setting the voltage sensor and low battery pin
  // Dropout voltage is 120 mV. So with a safety margin: 3.3 + .12 = 3.42 V
  // Fully charged the 3 AAA batteries provide 4.5 V
  pinMode(BAT_SENSE, INPUT);
  pinMode(LED_LOW_BAT, OUTPUT);
  // The multiply two comes from the voltage divider circuit for measuring voltages above 3.3V
  float battery_voltage = analogRead(BAT_SENSE)/float(4095)*float(3.3)*float(2);
  //Serial.print("Battery voltage: "); Serial.println(battery_voltage);
  //Serial.print("Analog Read: "); Serial.println(analogRead(BAT_SENSE));
  if(battery_voltage < 3.42) {
    Serial.println("LOW BATTERY WARNING!");
     Serial.print("Battery voltage: "); Serial.println(battery_voltage);
    digitalWrite(LED_LOW_BAT, HIGH);
  }
  else{
    digitalWrite(LED_LOW_BAT, LOW);
  }


  // Setting the LED to be controlled by the webserver
  pinMode(LED_WEBSERVER, OUTPUT);

  // Software serial settings and start
  ss.begin(GPSBAUD);

  // SPI bus settings and start
  SPI.begin(SCK, MISO, MOSI, SS_ALT);
  SPI.setClockDivider(SPI_CLOCK_DIV64);

  // Initialization prints
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
  initSDCard();

  String dataMessage = "Session identifier, " + String(session_identifier) + "\r\n";

  File file = SD.open("/ReceivedMessages.txt");
  if (!file)
  {
    Serial.println("File doesn't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/ReceivedMessages.txt", dataMessage.c_str());
  }
  else
  {
    appendFile(SD, "/ReceivedMessages.txt", dataMessage.c_str());
  }
  file.close();
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

  // Initializing static IP adress
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
  {
    Serial.println("STA Failed to configure");
  }

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
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", String(), false, processor); });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/style.css", "text/css"); });

  // Royute to load script.js file
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/script.js", "text/javascript"); });

  // Start ElegantOTA
  AsyncElegantOTA.begin(&server);
  // Start server
  server.begin();

  if(SD_present){
    ws.textAll("SDP");
  }
  else{
    ws.textAll("SDNP");
  }
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
  if (packetSize)
  {
    // received a packet
    Serial.print("Received packet '");

    // Read packet
    while (LoRa.available())
    {
      LoRaData = LoRa.readString();
      Serial.print(LoRaData);

      // print RSSI of packet
      Serial.print("' with RSSI ");
      LoRa_RSSI = LoRa.packetRssi();
      Serial.println(LoRa_RSSI);
    }

    String dataMessage = String(LoRaData) + "," + String(LoRa_RSSI) + "\r\n";
    Serial.print("Writing following message to SD: ");
    Serial.println(dataMessage);
    appendFile(SD, "/ReceivedMessages.txt", dataMessage.c_str());

    // Send the LoRa data to the HTML page
    // updateHTML_LoRa(LoRaData);
    ws.textAll(LoRaData);
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
    Serial.print(" In dec form:"); Serial.print(bite2);

    digitalWrite(SS_ALT, HIGH);
    delayMicroseconds(10);

    // Sending altdata to webpage
    Serial.print("Sending following to webpage: ");
    ws.textAll("Alt: " + String(bite2));

    // Send LoRa packet to receiver
    Serial.print("Sending packet: ");
    Serial.println(counter);

    digitalWrite(SS_LORA, LOW);
    LoRa.beginPacket();
    LoRa.print(String("ID " + NodeName + ":"));
    LoRa.print(session_identifier);
    LoRa.print(" packet_number: ");
    LoRa.print(counter);
    LoRa.endPacket();
    counter++;
    digitalWrite(SS_LORA, HIGH);
    delayMicroseconds(100);
    prev_time = millis();
    
    String GPS_time;
    String long_lat;
    String num_sat; 
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

      GPS_time = String(gps.time.hour()) +":" + String(gps.time.minute()) + ":" + String(gps.time.second());
      sprintf(buffer, "GPS time to be send: %s", GPS_time);
      Serial.println(buffer);
    }
    if (gps.satellites.isUpdated())
  {
    Serial.print(F("SATELLITES Fix Age="));
    Serial.print(gps.satellites.age());
    Serial.print(F("ms Value="));
    Serial.println(gps.satellites.value());

    num_sat = String(gps.satellites.value());
    sprintf(buffer, "GPS number of sattelites to be send: %s", num_sat);
      Serial.println(buffer);
  }
   if (gps.location.isUpdated())
  {
    Serial.println("waTCH THIS"); Serial.write(ss.read());
    Serial.print(F("LOCATION   Fix Age="));
    Serial.print(gps.location.age());
    Serial.print(F("ms Raw Lat="));
    Serial.print(gps.location.rawLat().negative ? "-" : "+");
    Serial.print(gps.location.rawLat().deg);
    Serial.print("[+");
    Serial.print(gps.location.rawLat().billionths);
    Serial.print(F(" billionths],  Raw Long="));
    Serial.print(gps.location.rawLng().negative ? "-" : "+");
    Serial.print(gps.location.rawLng().deg);
    Serial.print("[+");
    Serial.print(gps.location.rawLng().billionths);
    Serial.print(F(" billionths],  Lat="));
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(" Long="));
    Serial.println(gps.location.lng(), 6);

    long_lat = "Lat= " + String(gps.location.lat()) + " Long= " + String(gps.location.lng());
    sprintf(buffer, "GPS time to be send: %s", long_lat);
      Serial.println(buffer);
  }
   ws.textAll("GPS: " + GPS_time + " " + long_lat + " " + num_sat);

  }

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

void notifyClients()
{
  ws.textAll(String(ledState));
}

// To set the initial value of the button
String processor(const String &var)
{
  Serial.println(var);
  if (var == "STATE")
  {
    if (ledState)
    {
      return "ON";
    }
    else
    {
      return "OFF";
    }
  }
  return String();
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    if (strcmp((char *)data, "toggle") == 0)
    {
      ledState = !ledState;
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
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

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

// Initialize SD card
void initSDCard()
{
  if (!SD.begin(SS_SD))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  SD_present = 1;
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC)
  {
    Serial.println("MMC");
  }
  else if (cardType == CARD_SD)
  {
    Serial.println("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    Serial.println("SDHC");
  }
  else
  {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

// Write to the SD card
void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("File written");
  }
  else
  {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card
void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    Serial.println("Message appended");
  }
  else
  {
    Serial.println("Append failed");
  }
  file.close();
}

void updateHTML_LoRa(String LoRaMessage)
{
}