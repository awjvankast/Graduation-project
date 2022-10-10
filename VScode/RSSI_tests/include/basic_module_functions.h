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
#define SF_FACTOR 9

#define BAT_SENSE 35

#define LED_LOW_BAT 33
#define LED_WEBSERVER 32

#define SEND_PERIOD 500

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

#endif