// Set to 1 to enable debugging and printing, 0 for disabling 
#define DEBUG 1

// Used to enable or disable printing when debugging
#if DEBUG
#define D_print(...) Serial.print(__VA_ARGS__)
#define D_println(...) Serial.println(__VA_ARGS__)
#else
#define D_print(...) 
#define D_println(...) 
#endif

// Header guards
#ifndef BASIC_MODULE_FUNCTIONS
#define BASIC_MODULE_FUNCTIONS

#include <Arduino.h>

#include <LoRa.h>

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

#include "time.h"

#define SCK 18
#define MISO 23
#define MOSI 19

#define SS_LORA 15
#define SS_ALT 0
#define SS_SD 5

#define LORA_RESET 13
#define LORA_DATA 4
#define SYNC_LORA 0xF3

#define RX_GPS 39
#define TX_GPS 27
#define GPS_NC 36
#define PPS_GPS 34
#define STANDBY_GPS 22
#define RESET_GPS 2

#define GPSBAUD 9600
#define SF_FACTOR_INTER_GATEWAY 7
#define SF_FACTOR_TX_INTER 7

#define BAT_SENSE 35

#define LED_LOW_BAT 33
#define LED_WEBSERVER 32    

#define SEND_PERIOD 125
#define TX_WAIT_TIME 1000
#define MEASURE_PERIOD 100000
#define MAX_QUEUE 200
#define MAX_PACKET_NUMBER 360000
#define PACKETS_PER_PERIOD 8
#define NUMBER_OF_TRX_MODULES 6

// SD card functions
void initSDCard();
void writeFile(fs::FS &fs, const char *path, const char *message);
void appendFile(fs::FS &fs, const char *path, const char *message);

// Initialize all the pins with their respective functions, the SPI bus 
void pin_SPI_initialization();

// Initialize all the modules and print
void all_modules_initialization();

// Own functions for printing bytes to serial monitor
void printBin(byte aByte);
void printBin16(unsigned int aByte);

// Websocket functions
void initWebSocket();
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len);
String processor(const String &var);
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void notifyClients();

// Altimeter functions
unsigned int retrieve_altimeter_value();

// GPS functions
void check_GPS_time_loc_sat();

// Function for getting the time from NTP server
String LocalTimeToString();

#endif