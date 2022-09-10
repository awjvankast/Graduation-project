#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

#include <SPI.h>
#include <LoRa.h>

#define LEDPIN_BAT 32
#define LEDPIN 33

#define SCK 18 
#define MISO 23
#define MOSI 19

#define CS_LORA 15
#define RST_LORA 13
#define DATA_LORA 4
#define CS_ALT 0
#define CS_SD 5

int counter = 0;
int no_LoRa = 0;

const char* host = "esp32";
const char* ssid = "POCO";
const char* password = "knabobar";

WebServer server(80);

/*
   Login page
*/

const char* loginIndex =
  "<form name='loginForm'>"
  "<table width='20%' bgcolor='A09F9F' align='center'>"
  "<tr>"
  "<td colspan=2>"
  "<center><font size=4><b>ESP32 Login Page</b></font></center>"
  "<br>"
  "</td>"
  "<br>"
  "<br>"
  "</tr>"
  "<tr>"
  "<td>Username:</td>"
  "<td><input type='text' size=25 name='userid'><br></td>"
  "</tr>"
  "<br>"
  "<br>"
  "<tr>"
  "<td>Password:</td>"
  "<td><input type='Password' size=25 name='pwd'><br></td>"
  "<br>"
  "<br>"
  "</tr>"
  "<tr>"
  "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
  "</tr>"
  "</table>"
  "</form>"
  "<script>"
  "function check(form)"
  "{"
  "if(form.userid.value=='admin' && form.pwd.value=='admin')"
  "{"
  "window.open('/serverIndex')"
  "}"
  "else"
  "{"
  " alert('Error Password or Username')/*displays error message*/"
  "}"
  "}"
  "</script>";

/*
   Server Index Page
*/

const char* serverIndex =
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update'>"
    "<input type='submit' value='Update'>"
    "</form>"
    "<div id='prg'>progress: 0%</div>"
    "<script>"
    "$('form').submit(function(e){"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    " $.ajax({"
    "url: '/update',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData:false,"
    "xhr: function() {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener('progress', function(evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
    "}"
    "}, false);"
    "return xhr;"
    "},"
    "success:function(d, s) {"
    "console.log('success!')"
    "},"
    "error: function (a, b, c) {"
    "}"
    "});"
    "});"
  "</script>";

/*
   setup function
*/
void setup(void) {
  Serial.begin(115200);

  // Led pin enable
  pinMode(LEDPIN_BAT, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  
  digitalWrite(LEDPIN_BAT, LOW);   // turn the
  digitalWrite(LEDPIN, LOW);   // turn the


  // Chip initializing selects pins 
  pinMode(CS_SD, OUTPUT);
  pinMode(CS_SD, HIGH);

  pinMode(CS_ALT, OUTPUT);
  pinMode(CS_ALT, HIGH);

  pinMode(CS_LORA, OUTPUT);
  pinMode(CS_LORA, HIGH);

  SPI.begin(SCK, MISO, MOSI, CS_ALT);
  SPI.setClockDivider(SPI_CLOCK_DIV64);

  LoRa.setPins(CS_LORA, RST_LORA, DATA_LORA);

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("IT WORKED WOOOPOPOOPOW");

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });

  server.begin();

Serial.println("Pulling down LoRa");
  digitalWrite(CS_LORA, LOW);
  
Serial.println("Initializing LoRa");
  while (!LoRa.begin(433E6) && counter < 20) {
    Serial.print(counter); Serial.print(", ");
    counter++;
    delay(500);
  }
Serial.println("writing LoRa high");
  digitalWrite(CS_LORA, HIGH);

  if ( counter >= 20) {
    no_LoRa = 1;
    Serial.println(""); Serial.println("LoRa Initializing failed!");
  }
  else  {
    LoRa.setSyncWord(0xF3);
    Serial.println("LoRa Initializing OK!");
  }
  counter= 0;

  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF


}

void loop(void) {
  Serial.println("Made it to the loop");
  server.handleClient();
  delay(1);
  
 
  digitalWrite(LEDPIN, HIGH);  

  // Altimeter handling
  digitalWrite(CS_ALT,HIGH);
  delayMicroseconds(5);
  digitalWrite(CS_ALT, LOW);
  SPI.transfer(0x1E); //reset
  delay(3);
  digitalWrite(CS_ALT, HIGH);
  delayMicroseconds(100);
  digitalWrite(CS_ALT, LOW);
  delayMicroseconds(10);
  SPI.transfer(0xA4); //sending 8 bit command
  delayMicroseconds(20);
  unsigned int bite1 = SPI.transfer16(0x0000); // sending 0

  printBin16(bite1);

  digitalWrite(CS_ALT, HIGH);
  delay(10);
  Serial.println();


  // LoRa handling
  // tries to send a packets and increases the packet number every time it has been done
  if (no_LoRa == 0 && counter % 100 == 0) {
    digitalWrite(CS_LORA, LOW);
    Serial.print("Sending packet: ");
    Serial.println(counter);

    //Send LoRa packet to receiver
    LoRa.beginPacket();
    LoRa.print("hello ");
    LoRa.print(counter);
    LoRa.endPacket();

    digitalWrite(CS_LORA, HIGH);
  }

  counter++;
  delay(1000);

}

void printBin16(unsigned int aByte) {
  for (int8_t aBit = 15; aBit >= 0; aBit--)
    Serial.print(bitRead(aByte, aBit) ? '1' : '0');
  Serial.println();
}

void printBin(byte aByte) {
  for (int8_t aBit = 7; aBit >= 0; aBit--)
    Serial.print(bitRead(aByte, aBit) ? '1' : '0');
}
