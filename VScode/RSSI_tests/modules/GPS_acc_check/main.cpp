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
extern int loc_updated;
void loop()
{
  // Websocket update
  ws.cleanupClients();

    while (ss.available() > 0) gps.encode(ss.read());

    extern String GPS_time;
    extern String lat_long;
    extern String num_sat;

    check_GPS_time_loc_sat();
    
  if (loc_updated){
      D_print("Writing to SD... : ");
      D_println(lat_long);
      //String dataMes = lat_long + "\r\n";
      String dataMes = lat_long + ",\r\n";
      appendFile(SD, "/ReceivedMessages.txt", dataMes.c_str());
      loc_updated = 0;
    }

}















