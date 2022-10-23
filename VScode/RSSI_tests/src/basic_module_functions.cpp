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
String lat_long;
String num_sat;

void pin_SPI_initialization()
{
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
  float battery_voltage = analogRead(BAT_SENSE) / float(4095) * float(3.3) * float(2);
  D_print("Battery voltage: "); D_println(battery_voltage);
  D_print("Analog Read: "); D_println(analogRead(BAT_SENSE));
  if (battery_voltage < 3.42)
  {
    D_println(F("LOW BATTERY WARNING!"));
    D_print(F("Battery voltage: "));
    D_println(battery_voltage);
    digitalWrite(LED_LOW_BAT, HIGH);
  }
  else
  {
    digitalWrite(LED_LOW_BAT, LOW);
  }

  SPI.begin(SCK, MISO, MOSI, SS_ALT);
  SPI.setClockDivider(SPI_CLOCK_DIV64);
}

void all_modules_initialization()
{
  D_println(F("Starting all modules initialization"));
  // Set your Static IP address
  IPAddress local_IP(192, 168, 43, last_IP_number); // A = 1, B = 2, ...
  // Set your Gateway IP address
  IPAddress gateway(192, 168, 43, last_IP_number);
  IPAddress subnet(255, 255, 0, 0);
  IPAddress primaryDNS(8, 8, 8, 8);   // optional
  IPAddress secondaryDNS(8, 8, 4, 4); // optional

  session_identifier = random(11111, 99999);
  D_print(F("Session identifier: "));
  D_println(session_identifier);
  D_println("");

  D_println(F("--- LORA INITIALIZAITON ---"));
  digitalWrite(SS_LORA, LOW);
  LoRa.setPins(SS_LORA, LORA_RESET, LORA_DATA);
  while (!LoRa.begin(433E6))
  {
    D_print(".");
    delay(500);
  }
  LoRa.setSyncWord(SYNC_LORA);
  D_println("");
  D_println(F("LoRa initialized!"));
  LoRa.setSpreadingFactor(SF_FACTOR_TX_INTER);
  D_println();
  digitalWrite(SS_LORA, HIGH);

  D_println(F("--- SD INITIALIZATION ---"));
  initSDCard();

  String dataMessage = "Session identifier," + String(session_identifier) + "\r\n";

  File file = SD.open("/ReceivedMessages.txt");
  if (!file)
  {
    D_println(F("File doesn't exist"));
    D_println(F("Creating file..."));
    writeFile(SD, "/ReceivedMessages.txt", dataMessage.c_str());
  }
  else
  {
    appendFile(SD, "/ReceivedMessages.txt", dataMessage.c_str());
  }
  file.close();
  D_println();

  D_println(F("--- SPIFFS INITIALIZATION ---"));
  while (!SPIFFS.begin(true))
  {
    D_print(".");
    delay(500);
  }
  D_println("");
  D_println(F("SPIFFS initialized!"));
  D_println();

  // Connect to Wi-Fi
  D_println(F("--- WIFI INITIALIZATION ---"));

  // Initializing static IP adress
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
  {
    D_println(F("STA Failed to configure"));
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    D_print(".");
  }
  initWebSocket();
  D_println("");
  D_println(F("Wi-Fi initialized!"));

  // Print ESP32 Local IP Address
  D_print(F("IP adress: "));
  D_println(WiFi.localIP());
  D_println();

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
    D_print(bitRead(aByte, aBit) ? '1' : '0');
  D_println();
}

void printBin16(unsigned int aByte)
{
  for (int8_t aBit = 15; aBit >= 0; aBit--)
    D_print(bitRead(aByte, aBit) ? '1' : '0');
  D_println();
}

void initSDCard()
{
  if (!SD.begin(SS_SD))
  {
    D_println(F("Card Mount Failed"));
    return;
  }
  SD_present = 1;
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    D_println(F("No SD card attached"));
    return;
  }
  D_print(F("SD Card Type: "));
  if (cardType == CARD_MMC)
  {
    D_println(F("MMC"));
  }
  else if (cardType == CARD_SD)
  {
    D_println(F("SDSC"));
  }
  else if (cardType == CARD_SDHC)
  {
    D_println(F("SDHC"));
  }
  else
  {
    D_println(F("UNKNOWN"));
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
    D_println(F("Failed to open file for writing"));
    return;
  }
  if (file.print(message))
  {
    D_println(F("File written"));
  }
  else
  {
    D_println(F("Write failed"));
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
    D_println(F("Failed to open file for appending"));
    return;
  }
  if (file.print(message))
  {
    D_println(F("Message appended"));
  }
  else
  {
    D_println(F("Append failed"));
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
  D_println(var);
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

unsigned int retrieve_altimeter_value()
{
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
  D_print(F("Byte from reading PROM altimeter: "));
  printBin16(bite2);
  D_print(F(" In dec form:"));
  D_print(bite2);

  digitalWrite(SS_ALT, HIGH);
  delayMicroseconds(10);

  return bite2;
}

void check_GPS_time_loc_sat()
{

  if (gps.time.isUpdated())
  {
    D_print(F("TIME       Fix Age="));
    D_print(gps.time.age());
    D_print(F("ms Raw="));
    D_print(gps.time.value());
    D_print(F(" Hour="));
    D_print(gps.time.hour());
    D_print(F(" Minute="));
    D_print(gps.time.minute());
    D_print(F(" Second="));
    D_print(gps.time.second());
    D_print(F(" Hundredths="));
    D_println(gps.time.centisecond());

    GPS_time = String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()) + ":" + String(gps.time.centisecond());
    sprintf(buffer, "GPS time to be send: %s", GPS_time);
    D_println(buffer);
  }
  if (gps.satellites.isUpdated())
  {
    D_print(F("SATELLITES Fix Age="));
    D_print(gps.satellites.age());
    D_print(F("ms Value="));
    D_println(gps.satellites.value());

    num_sat = String(gps.satellites.value());
    sprintf(buffer, "GPS number of sattelites to be send: %s", num_sat);
    D_println(buffer);
  }
  if (gps.location.isUpdated())
  {
    D_print(F("LOCATION   Fix Age="));
    D_print(gps.location.age());
    D_print(F("ms Raw Lat="));
    D_print(gps.location.rawLat().negative ? "-" : "+");
    D_print(gps.location.rawLat().deg);
    D_print("[+");
    D_print(gps.location.rawLat().billionths);
    D_print(F(" billionths],  Raw Long="));
    D_print(gps.location.rawLng().negative ? "-" : "+");
    D_print(gps.location.rawLng().deg);
    D_print("[+");
    D_print(gps.location.rawLng().billionths);
    D_print(F(" billionths],  Lat="));
    D_print(gps.location.lat());
    D_print(F(" Long="));
    D_println(gps.location.lng());

    lat_long = String(gps.location.rawLat().deg) + "." + String(gps.location.rawLat().billionths) + "," 
    + String(gps.location.rawLng().deg) + "." + String(gps.location.rawLng().billionths );
    sprintf(buffer, "GPS time to be send: %s", lat_long);
    D_println(buffer);
  }
}