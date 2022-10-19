// GATEWAY NODE BASIC RSSI TEST
// Code to receive data from the Tx and the intermediate nodes. Should save the data on the SD card and preferably also show it on a webpage.

// TODO
// - Implement library to get the altimeter values from the SPI bus
// - Low power implementation LED X
// - set spreading factor better -> set to 9 now (range 7-12), increase in SF is higher range but lower data and higher power consumption
// - Set the correct form for GPS to increase accuracy and decrease speed
// - Adjust Javascript for correct printing to webpage X

#include "basic_module_functions.h"

// CHANGE THIS for every different node
String NodeName = "Honnold";
int last_IP_number = 8;

bool ledState = 0;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// The TinyGPSPlus object and initializing software serial
TinyGPSPlus gps;
SoftwareSerial ss(RX_GPS, TX_GPS);

File myFile;

int packet_number = 0;
extern unsigned long session_identifier;
unsigned long last = 0UL;
extern int SD_present;

void setup()
{
  // Standard baud rate of ESP32
  Serial.begin(115200);

  pin_SPI_initialization();
  ss.begin(GPSBAUD);

  D_println("------------- RSSI test gateway node -------------");
  all_modules_initialization();
  LoRa.setSpreadingFactor(SF_FACTOR_INTER_GATEWAY);
}

unsigned long prev_time = millis()+ MEASURE_PERIOD;


void loop()
{
  // Websocket update
  ws.cleanupClients();

  // Led toggling for test purposes
  digitalWrite(LED_WEBSERVER, ledState);

  // Dispatch incoming GPS characters
  while (ss.available() > 0)
    gps.encode(ss.read());

  if (millis() - MEASURE_PERIOD > prev_time)
  {
    extern String GPS_time;
    extern String lat_long;
    extern String num_sat;

    check_GPS_time_loc_sat();

    // Check a value of the altimeter
    unsigned int bite2 = retrieve_altimeter_value();

    // Send GPS data to HTML page
    ws.textAll("Z: " + GPS_time + " " + lat_long + " " + num_sat);
    // Sending altdata to webpage
    ws.textAll("Q: " + String(bite2));

    prev_time = millis();
  }

  // Checking for incoming messages from LoRa module
  if (LoRa.parsePacket())
  {
    String LoRaData;
    int LoRa_RSSI;

    // If a packet is received, continue to read the data and output it to the
    // serial monitor and the SD card

    // received a packet
    D_print("Received packet ");

    // Read packet
    while (LoRa.available())
    {
      LoRaData = LoRa.readString();
      D_print(LoRaData);

      // print RSSI of packet
      D_print(" with RSSI ");
      LoRa_RSSI = LoRa.packetRssi();
      D_println(LoRa_RSSI);
    }

    // Check if the received message is from the Tx node, Alex in this case
    // if so, save it to SD
    if (LoRaData.charAt(0) >= 'B' && LoRaData.charAt(0) <= 'G')
    {
      digitalWrite(LED_WEBSERVER,HIGH);
      // Implement timestamping here
      String dataMessage = String( LoRaData + "\r\n");
      appendFile(SD, "/ReceivedMessages.txt", dataMessage.c_str());

      // Send the LoRa data to the HTML page
      D_print(dataMessage);
      ws.textAll(LoRaData);
      digitalWrite(LED_WEBSERVER,LOW);
    }
  }
}
