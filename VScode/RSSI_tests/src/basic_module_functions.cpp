#include "basic_module_functions.h"
#include <AsyncElegantOTA.h> // needs to be here because of shitty library code practice, cannot include the library in .h file without getting errors

int SD_present = 0;
unsigned long session_identifier = 0;
extern int last_IP_number;

const char *ssid = "POCO";
const char *password = "knabobar";

extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern TinyGPSPlus gps;

extern int ledState;

char buffer[50];

    String GPS_time;
    String long_lat;
    String num_sat;

void pin_SPI_initialization(){
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

  pinMode(LED_WEBSERVER, OUTPUT);

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

  SPI.begin(SCK, MISO, MOSI, SS_ALT);
  SPI.setClockDivider(SPI_CLOCK_DIV64);
}

void all_modules_initialization(){
  Serial.println("Starting all modules initialization");
    // Set your Static IP address
    IPAddress local_IP(192, 168, 43, last_IP_number); // A = 1, B = 2, ...
  // Set your Gateway IP address
  IPAddress gateway(192, 168, 43, last_IP_number);
  IPAddress subnet(255, 255, 0, 0);
  IPAddress primaryDNS(8, 8, 8, 8);   // optional
  IPAddress secondaryDNS(8, 8, 4, 4); // optional

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
  LoRa.setSpreadingFactor(SF_FACTOR);
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
    delay(100);
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

  if (SD_present)
  {
    ws.textAll("SDP");
  }
  else
  {
    ws.textAll("SDNP");
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

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
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

void notifyClients()
{
  ws.textAll(String(ledState));
}

unsigned int retrieve_altimeter_value(){
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
    Serial.print(" In dec form:");
    Serial.print(bite2);

    digitalWrite(SS_ALT, HIGH);
    delayMicroseconds(10);

    return bite2;
}

  void check_GPS_time_loc_sat(){

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

      GPS_time = String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second());
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
      Serial.print(gps.location.lat());
      Serial.print(F(" Long="));
      Serial.println(gps.location.lng());

      long_lat = "Lat= " + String(gps.location.lat()) + " Long= " + String(gps.location.lng());
      sprintf(buffer, "GPS time to be send: %s", long_lat);
      Serial.println(buffer);
    }
  }