// Tx implementation for basic RRSI test
// DESCRIPTION
// The Tx node only sends data over LoRa from it's own sensors and it's own ID

// TODO
// - Implement library to get the altimeter values from the SPI bus

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

int packet_number = 1;
extern unsigned long session_identifier;
unsigned long last = 0UL;
extern int SD_present;

void setup()
{
  // Standard baud rate of ESP32
  Serial.begin(115200);

  pin_SPI_initialization();
  ss.begin(GPSBAUD);

  D_println("------------- RSSI test Transmitter node -------------");
  all_modules_initialization();

}

unsigned long prev_time = millis();

void loop()
{
  // Websocket update
  ws.cleanupClients();

  // Led toggling for test purposes
  digitalWrite(LED_WEBSERVER, ledState);

  if ( (packet_number - 1) % PACKETS_PER_PERIOD  == 0) {
    unsigned long wait_tracker = millis();
    digitalWrite(LED_WEBSERVER,LOW);

    // Do some measurements while waiting
    // Dispatch incoming GPS characters
    while (ss.available() > 0) gps.encode(ss.read());

    // Check a value of the altimeter and get the GPS data if available
    unsigned int bite2 = retrieve_altimeter_value();
    extern String GPS_time;
    extern String lat_long;
    extern String num_sat;
    check_GPS_time_loc_sat();
    
    // Sending alt data and GPS data to webpage
    ws.textAll("Z: " + GPS_time + " " + lat_long + " " + num_sat);
    D_print("Sending following altdata to webpage: ");
    ws.textAll("Q: " + String(bite2));

    // Wait untill the wait time has passed
    while(millis() < wait_tracker + TX_WAIT_TIME) 1;
  }

  // Checking for incoming messages from LoRa module
  if(millis() - SEND_PERIOD > prev_time && millis() > SEND_PERIOD){
    // Send LoRa packet to receiver
    prev_time = millis();
    D_print("Sending packet: ");

    // Composing the packet, sending it and using the LED as indicator
    String packet_send_now = String(String(NodeName.charAt(0)) + "," + String(session_identifier) + "," + String(packet_number) );
    digitalWrite(LED_WEBSERVER,HIGH);
    LoRa.beginPacket();
    LoRa.print( packet_send_now );
    LoRa.endPacket();
    digitalWrite(LED_WEBSERVER,LOW);
    
    // Printing to console and HTML page
    D_println( packet_send_now );
    ws.textAll( packet_send_now );

    // Increase packet number and reset the timer
    packet_number++;

  }
}















