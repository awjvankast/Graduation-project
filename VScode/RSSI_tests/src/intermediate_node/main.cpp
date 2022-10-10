// INTERMEDIATE NODE BASIC RSSI TEST
// Code to receive data from Tx by LoRa. This data together with the RSSI values is stored and then forwarded to the gateway node.
// The forwarded message also contains data from the altimeter of the intermediate node.

// TODO
// - Implement library to get the altimeter values from the SPI bus
// - Low power implementation LED X
// - set spreading factor better -> set to 9 now (range 7-12), increase in SF is higher range but lower data and higher power consumption
// - Set the correct form for GPS to increase accuracy and decrease speed 
// - Adjust Javascript for correct printing to webpage

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

int counter = 0;
extern unsigned long session_identifier;
unsigned long last = 0UL;
extern int SD_present;

void setup()
{
  // Standard baud rate of ESP32
  Serial.begin(115200);

  pin_SPI_initialization();
  ss.begin(GPSBAUD);

  Serial.println("------------- RSSI test intermediate node -------------");
  all_modules_initialization();

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
  if(LoRa.parsePacket()){

    String LoRaData;
    int LoRa_RSSI;

  // If a packet is received, continue to read the data and output it to the
  // serial monitor and the SD card
  
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

    // Check if the received message is from the Tx node, Alex in this case
    if (LoRaData.charAt(1) == 'A')
    {

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

    // Check a value of the altimeter
    unsigned int bite2 = retrieve_altimeter_value();

    extern String GPS_time;
    extern String long_lat;
    extern String num_sat;

    check_GPS_time_loc_sat();
    
    ws.textAll("GPS: " + GPS_time + " " + long_lat + " " + num_sat);
  
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

    LoRa.print("Own data \r\n");
    LoRa.print(" GPS time:"); LoRa.print(GPS_time);
    LoRa.print(", GPS location:"); LoRa.print(long_lat);
    LoRa.print(", Number of sat:"); LoRa.print(num_sat);

    LoRa.print("Tx data \r\n");
    LoRa.print(LoRaData);

    LoRa.endPacket();
    counter++;
    digitalWrite(SS_LORA, HIGH);
    delayMicroseconds(100);
    prev_time = millis();

  }
}












