// Tx implementation for basic RRSI test
// DESCRIPTION
// The Tx node only sends data over LoRa from it's own sensors and it's own ID

// TODO
// - Implement library to get the altimeter values from the SPI bus
// - Low power implementation LED X
// - set spreading factor better X  

#include "basic_module_functions.h"

// CHANGE THIS for every different node
String NodeName = "Alex";
int last_IP_number = 1;

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

  Serial.println("------------- RSSI test Transmitter node -------------");
  all_modules_initialization();

}

unsigned long prev_time = millis()+SEND_PERIOD;

void loop()
{
  // Websocket update
  ws.cleanupClients();

  // Led toggling for test purposes
  digitalWrite(LED_WEBSERVER, ledState);

  // Checking for incoming messages from LoRa module
  if(millis() - SEND_PERIOD > prev_time){

    // Dispatch incoming GPS characters
    while (ss.available() > 0)
      gps.encode(ss.read());

    // Check a value of the altimeter
    unsigned int bite2 = retrieve_altimeter_value();

    extern String GPS_time;
    extern String lat_long;
    extern String num_sat;

    check_GPS_time_loc_sat();
    
    ws.textAll("GPS: " + GPS_time + " " + lat_long + " " + num_sat);
  
    // Sending altdata to webpage
    Serial.print("Sending following to webpage: ");
    ws.textAll("Alt: " + String(bite2));

    // Send LoRa packet to receiver
    Serial.print("Sending packet: ");
    Serial.println(packet_number);

    digitalWrite(SS_LORA, LOW);
    LoRa.beginPacket();
    LoRa.print( String(NodeName + ", " + String(session_identifier) + ", " + String(packet_number)) );

    LoRa.endPacket();
    packet_number++;
    digitalWrite(SS_LORA, HIGH);
    delayMicroseconds(100);
    prev_time = millis();

  }
}












