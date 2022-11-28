// INTERMEDIATE NODE BASIC RSSI TEST

#include "basic_module_functions.h"

// CHANGE THIS for every different node
String NodeName = "Brooke";
int last_IP_number = 2;

bool ledState = 0;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// The TinyGPSPlus object and initializing software serial
TinyGPSPlus gps;
SoftwareSerial ss(RX_GPS, TX_GPS);

File myFile;

extern unsigned long session_identifier;
unsigned long last = 0UL;
extern int SD_present;

unsigned int bite2;

void setup()
{
  // Standard baud rate of ESP32
  Serial.begin(115200);

  pin_SPI_initialization();
  ss.begin(GPSBAUD);

  D_println("------------- RSSI test intermediate node -------------");
  all_modules_initialization();
}

unsigned long prev_time = millis();
unsigned long time_before_next_send = 429496729;
// Check how many packets untill nex send period of this node and adjust countdown
int packets_remaining_before_send;
unsigned long packet_send_timestamp;
int add_dataHeader = 1;
int packets_in_send_queue = 0;
int packet_number = 1;
int send_interval_counter = 1;
String dataHeader;
String dataBody;
String dataTotal;

// Checking GPS data
extern String GPS_time;
extern String lat_long;
extern String num_sat;

void loop()
{
  // Websocket update
  ws.cleanupClients();

  // Led toggling for test purposes
  digitalWrite(LED_WEBSERVER, ledState);

  // Dispatch incoming GPS characters
  while (ss.available() > 0)
    gps.encode(ss.read());

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

      digitalWrite(LED_WEBSERVER,HIGH);

      //D_print("Received message from intermediate node");
      String dataMessage = String( LoRaData + "," + LoRa_RSSI + "\r\n");
      //D_print("Printing the length of the string received: "); D_println(LoRaData.length());
      appendFile(SD, "/ReceivedMessages.txt", dataMessage.c_str());
      // Send the LoRa data to the HTML page
      //D_print(dataMessage);
      //ws.textAll(LoRaData);
      digitalWrite(LED_WEBSERVER,LOW);
  }
}


