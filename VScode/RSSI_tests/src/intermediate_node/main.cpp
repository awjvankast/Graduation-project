// INTERMEDIATE NODE BASIC RSSI TEST
// Code to receive data from Tx by LoRa. This data together with the RSSI values is stored and then forwarded to the gateway node.
// The forwarded message also contains data from the altimeter of the intermediate node.

// TODO
// - Implement library to get the altimeter values from the SPI bus
// - Low power implementation LED X
// - set spreading factor better -> set to 9 now (range 7-12), increase in SF is higher range but lower data and higher power consumption
// - Set the correct form for GPS to increase accuracy and decrease speed
// - Adjust Javascript for correct printing to webpage X
// - Send geolocation as well
// - Don't use delay but millis()


#include "basic_module_functions.h"

// CHANGE THIS for every different node
String NodeName = "Gruber";
int last_IP_number = 7; 

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

  Serial.println("------------- RSSI test intermediate node -------------");
  all_modules_initialization();
}

unsigned long prev_time = millis();

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
    // Serial.print("Received packet ");

    // Read packet
    while (LoRa.available())
    {
      LoRaData = LoRa.readString();
      Serial.print(LoRaData);

      // print RSSI of packet
      Serial.print("' with RSSI ");
      LoRa_RSSI = LoRa.packetRssi();
      Serial.println(LoRa_RSSI);
      //ws.textAll(LoRaData);
    }

    // Check if the received message is from the Tx node, Alex in this case
    // if so, save it to SD
    if (LoRaData.charAt(0) == 'A')
    {
      delay(125 * (last_IP_number-2) );
      String dataMessage = String(LoRaData) + "," + String(LoRa_RSSI);
      appendFile(SD, "/ReceivedMessages.txt", dataMessage.c_str());

      // Check a value of the altimeter
      unsigned int bite2 = retrieve_altimeter_value();

      extern String GPS_time;
      extern String lat_long;
      extern String num_sat;

      //check_GPS_time_loc_sat();

      // Send LoRa packet to receiver
      Serial.println("Sending packet: ");
      Serial.println(packet_number);

      LoRa.setSpreadingFactor(7);
      digitalWrite(SS_LORA, LOW);
      LoRa.beginPacket();

      // See notes for the data format
      String LoRa_send_packet = String(String(NodeName.charAt(0)) + "," + String(session_identifier) + "," + String(packet_number) + "," +
                                       String(GPS_time) + "," + String(lat_long) + "," + String(num_sat) + "," + LoRaData + "," +
                                       String(LoRa_RSSI));

      LoRa.print(LoRa_send_packet);
      LoRa.endPacket();
      LoRa.setSpreadingFactor(8);

      packet_number++;
      digitalWrite(SS_LORA, HIGH);
      delayMicroseconds(100);

      Serial.print("LoRa packet send: ");
      Serial.println(LoRa_send_packet);
      // Send the LoRa data to the HTML page
      ws.textAll(LoRaData);
      // Send GPS data to HTML page
      //ws.textAll("Z: " + GPS_time + " " + lat_long + " " + num_sat);
      // Sending altdata to webpage
      //ws.textAll("Q: " + String(bite2));

      prev_time = millis();
    }
  }
}
