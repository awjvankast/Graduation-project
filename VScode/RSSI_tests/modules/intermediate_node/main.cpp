// INTERMEDIATE NODE BASIC RSSI TEST
// Code to receive data from Tx by LoRa. This data together with the RSSI values is stored and then forwarded to the gateway node.
// The forwarded message also contains data from the altimeter of the intermediate node.

// TODO
// - Implement library to get the altimeter values from the SPI bus
// - Low power implementation LED X
// - set spreading factor better -> set to 7 now (range 7-12), increase in SF is higher range but lower data and higher power consumption
// - Set the correct form for GPS to increase accuracy and decrease speed

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

unsigned int bite2;

void setup()
{
  // Standard baud rate of ESP32
  Serial.begin(115200);

  pin_SPI_initialization();
  ss.begin(GPSBAUD);

  D_println("------------- RSSI test intermediate node -------------");
  all_modules_initialization();
  bite2 = retrieve_altimeter_value();
}

unsigned long prev_time = millis();
int add_dataHeader = 1;
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
  
  // Do something at time intervals
  if (millis() - MEASURE_PERIOD > prev_time && millis() > MEASURE_PERIOD ){
      check_GPS_time_loc_sat();
      prev_time = millis();
  }

  // Checking for incoming messages from LoRa module
  if (LoRa.parsePacket())
  {
    String LoRaData;
    int LoRa_RSSI;
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
      // ws.textAll(LoRaData);
    }

    // Check if the received message is from the Tx node, Alex in this case
    // if so, save it to SD
    if (LoRaData.charAt(0) == 'A')
    { 
      digitalWrite(LED_WEBSERVER, HIGH);

      String dataMessage = String(LoRaData) + "," + String(LoRa_RSSI) + "\r\n";

      String Tx_ID = LoRaData.substring(2,6);
      int Tx_packet_number = LoRaData.substring(8).toInt();
      String Tx_data = dataMessage.substring(8);

      if (add_dataHeader)
      {
        add_dataHeader = 0;

        // See notes for the data format
        dataHeader = String(String(NodeName.charAt(0)) + "," + String(session_identifier) + "," + String(packet_number) + "," +
                            String(GPS_time) + "," + String(lat_long) + "," + String(num_sat) + "," + "A" + "," + Tx_ID + "\r\n" );
      }
      
      // Append data to the end of the String
      String dataBody_temp = dataBody + Tx_data ;
      dataBody = dataBody_temp;
      
      // With 20 packets spacing, send a new packet 
      // Packets have 3 packets spacing this way (375ms with 125ms Tx spacing)
      if( Tx_packet_number % ( last_IP_number + 18 + (last_IP_number-2)*2 ) == 0 && Tx_packet_number >= 20){
        packet_number++;
        D_println(F("Sending accumulated data"));
      
        dataTotal = dataHeader + dataBody;

        LoRa.setSpreadingFactor(SF_FACTOR_INTER_GATEWAY);
        LoRa.beginPacket();
        LoRa.print( dataTotal );
        LoRa.endPacket();
        LoRa.setSpreadingFactor(SF_FACTOR_TX_INTER);

        appendFile(SD, "/ReceivedMessages.txt", dataTotal .c_str());

        dataBody = "";
        dataHeader = "";
        add_dataHeader = 1;

        D_println("LoRa packet send: ");
        D_print(dataTotal);
      }
      // Send the LoRa data to the HTML page
      ws.textAll(LoRaData);

      digitalWrite(LED_WEBSERVER, LOW);
    }
  }
}
