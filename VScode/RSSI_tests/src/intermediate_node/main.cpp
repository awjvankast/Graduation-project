// INTERMEDIATE NODE BASIC RSSI TEST
// Code to receive data from Tx by LoRa. This data together with the RSSI values is stored and then forwarded to the gateway node.
// The forwarded message also contains data from the altimeter of the intermediate node.

// TODO
// - Implement library to get the altimeter values from the SPI bus
// - Low power implementation LED X
// - set spreading factor better -> set to 7 now (range 7-12), increase in SF is higher range but lower data and higher power consumption
// - Set the correct form for GPS to increase accuracy and decrease speed

// Potential causes failed experiment on hockey field
// - Not enough delays because of reflections?
// - Not connecting to wifi at startup FIXED
// - Data overflow from packets FIXED
// - Data not reaching gateway
// - need more sd cards for debugging
// - Use SNR?
// - Likely cause: collision of packets with different SF's. SF's apparently collide when the power difference is too big.

// Fix small bug with turning on/off transmitter and package delay

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

  // Do something at time intervals
  if (millis() - MEASURE_PERIOD > prev_time && millis() > MEASURE_PERIOD)
  {
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
      packets_in_send_queue++;

      String dataMessage = String(LoRaData) + "," + String(LoRa_RSSI) + "\r\n";

      String Tx_ID = LoRaData.substring(2, 6);
      int Tx_packet_number = LoRaData.substring(8).toInt();
      String Tx_data = dataMessage.substring(8);

      if (add_dataHeader)
      {
        add_dataHeader = 0;

        // See notes for the data format
        dataHeader = String(String(NodeName.charAt(0)) + "," + String(session_identifier) + "," + String(packet_number) + "," +
                            String(GPS_time) + "," + String(lat_long) + "," + String(num_sat) + "," + "A" + "," + Tx_ID + "\r\n");
      }

      // Append data to the end of the String
      String dataBody_temp = dataBody + Tx_data;
      dataBody = dataBody_temp;

      // Check if packet number is a reasonable number
      if (Tx_packet_number >= 0 && Tx_packet_number <= MAX_PACKET_NUMBER)
      {               
        // This evaluates  we currently how far untill the next send period
        // e.g. with PACKETS_PER_PERIOD = 8 and NUMBER_OF_TRX_MODULES = 6:
        // module B want to send after received packet [8, 56, 104, 152, ...]
        // The evaluation performs Tx_packet_number - [8, 56, ...] untill the result is negative
        // Then we know that, for instance, Tx_packet_number = 54 the next send period will take place in 56-54 = 2 packets
        // Tx should send a packet every SEND_PERIOD seconds so the delay can be set at 200ms
        
        int next_send_time_in_Tx_packets = (send_interval_counter - 1) * PACKETS_PER_PERIOD * NUMBER_OF_TRX_MODULES + PACKETS_PER_PERIOD * (last_IP_number - 1);
        
        // If the Tx_packet_number - next send time > 0 it means that some of the packets send by Tx are missed by the intermediate node
        // In this case loop over the interval counter untill the correct send_interval_counter is found
        if (Tx_packet_number - next_send_time_in_Tx_packets > 0)
        {
          D_print("Increasing interval counter due to missed Tx packets to ");
          while (Tx_packet_number - next_send_time_in_Tx_packets > 0)
          {
            send_interval_counter++;
            next_send_time_in_Tx_packets = (send_interval_counter - 1) * PACKETS_PER_PERIOD * NUMBER_OF_TRX_MODULES + PACKETS_PER_PERIOD * (last_IP_number - 1);
            D_print(". ");
          }
          D_println(send_interval_counter);
        }
        // If the amount of packages still needed to be received before the next send period is larger than possible in one period
        // Adjust the send_interval_counter accordingly
        else if(Tx_packet_number - next_send_time_in_Tx_packets < -PACKETS_PER_PERIOD * NUMBER_OF_TRX_MODULES){
          D_print("Decreasing interval counter due to restarting Tx to ");
          while (Tx_packet_number - next_send_time_in_Tx_packets < -PACKETS_PER_PERIOD * NUMBER_OF_TRX_MODULES)
          {
            send_interval_counter--;
            next_send_time_in_Tx_packets = (send_interval_counter - 1) * PACKETS_PER_PERIOD * NUMBER_OF_TRX_MODULES + PACKETS_PER_PERIOD * (last_IP_number - 1);
            D_print(". ");
          }
          D_println(send_interval_counter);
        }

        packets_remaining_before_send = next_send_time_in_Tx_packets - Tx_packet_number;
        time_before_next_send = packets_remaining_before_send * SEND_PERIOD;
        packet_send_timestamp = millis();

        D_print("Packets remaining before send: ");
        D_println(packets_remaining_before_send);
        D_print("Time before next send: ");
        D_println(time_before_next_send);
        D_print("packet send timestamp: ");
        D_println(packet_send_timestamp);
      }
    }
    // If buffer is too full, delete all data
    if (packets_in_send_queue >= MAX_QUEUE)
    {
      D_println(F("Deleting data queue because of overflow risk"));
      packets_in_send_queue = 0;
      dataBody = "";
      dataHeader = "";
      add_dataHeader = 1;
    }
    // Send the LoRa data to the HTML page
    ws.textAll(LoRaData);

    digitalWrite(LED_WEBSERVER, LOW);
  }
  // now start countdown

  // With 20 packets spacing, send a new packet
  // Packets have 3~7 packets spacing this way
  if (time_before_next_send == 0 || millis() >= time_before_next_send + packet_send_timestamp)
  {
    digitalWrite(LED_WEBSERVER,HIGH);

    time_before_next_send = 429496729;
    packet_number++;
    send_interval_counter++;
    packets_in_send_queue = 0;
    D_println(F("Sending accumulated data"));

    dataTotal = dataHeader + dataBody;

    D_print("Printing the length of the string to be send: "); D_println(dataTotal);
    
    LoRa.setSpreadingFactor(SF_FACTOR_INTER_GATEWAY);
    LoRa.beginPacket();
    LoRa.print(dataTotal);
    LoRa.endPacket();
    LoRa.setSpreadingFactor(SF_FACTOR_TX_INTER);

    appendFile(SD, "/ReceivedMessages.txt", dataTotal.c_str());

    dataBody = "";
    dataHeader = "";
    add_dataHeader = 1;

    D_println("LoRa packet send: ");
    D_print(dataTotal);
    digitalWrite(LED_WEBSERVER,LOW);
  }
}
