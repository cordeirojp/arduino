//
// BillBoard Arduino ESP8266 Code
// (C) ESP Porto Team #3 2018 & Damien Cordeiro
//

// Libraries for Wifi & HTTP
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Libraries for I2C and OLED Display
#include <Wire.h>
#include "SSD1306Wire.h"

// Libraries for Temperature Sensor
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// Wifi Logo BitMap
#include "images.h"

// Wifi Credentials
#define ssid      "Wifi2G"       // WiFi SSID
#define password  "DamThom9700"  // WiFi password

// GPIO for Presence Sensor
#define TRIGGER 16
#define ECHO    0
#define DISTANCE 50   // distance detect (cms)

// GPIO for LEDs
#define NB_BILLS 3
int led_Bills[4] = { 0, 0, 12, 13 }; // Array with the GPIO of each BillBoard


// IoT Cloud ThingSpeak
String apiKey = "IEM3NVQ01NWEX52E";           // API Key for BillBoard 1
const char* server = "api.thingspeak.com";    // IoTCloud URL
long billChannel[4] = { 0,0,469794,469795 };  // IoT Channels

// Pollution Limit
#define POLLUTION_LIMIT 250

// Global Variables
float sensorValue;                  // PPM Value
float myTemperature;                  // Temperature
float myAltitude;                     // Altitude
int Active;                         // Presence
WiFiClient client;                  // HTTP Object
SSD1306Wire display(0x3c, 4, 5);    // OLED Object (Address, SDA Pin, SDL Pin)
Adafruit_BMP280 bme;                // BMP Object

//
// Function : Connection WiFi
//
void ConnectWifi() {
  int counter = 0;

  // Clear Display
  display.clear();
  // Draw WiFi Logo
  display.drawXbm(0, 0, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  // Flush Display
  display.display();

  WiFi.begin ( ssid, password );
  while ( WiFi.status() != WL_CONNECTED ) {
    counter += 20;
    int progress = (counter / 5) % 100;
    display.drawProgressBar(0, 40, 120, 10, progress);
    display.display();
    delay ( 50 );
  }
}

//
// Function : Connection BMP
//
void ConnectBMP280() {
  Serial.println(F("BMP280 test"));
  if (!bme.begin()) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
  Serial.println(F("BMP280 OK"));
}

//
// Function : Get Distance Sensor
//
long GetDistance() {
  long duration;

  digitalWrite(TRIGGER, LOW);
  delay(2);
  digitalWrite(TRIGGER, HIGH);
  delay(10);
  digitalWrite(TRIGGER, LOW);
  duration = pulseIn(ECHO, HIGH);
  return (duration/2) / 29.1;
}

//
// Function : Post Data to IoT Cloud
//
void PostCloud() {
  Serial.println("PostCloud Start");
  if (client.connect(server,80))
    {
        String postStr = apiKey;
        postStr +="&field1=";
        postStr += String(sensorValue);
        postStr +="&field2=";
        postStr += String(myTemperature);
        postStr +="&field3=";
        postStr += String(myAltitude);
        postStr +="&field4=";
        postStr += String(Active);
        postStr += "\r\n\r\n";

        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(postStr.length());
        client.print("\n\n");
        client.print(postStr);
    }
  client.stop();
  Serial.println("PostCloud Stop");
  delay(100);
}

//
// Function Get Data of others BillBoards from IoT Cloud
// https://api.thingspeak.com/channels/469795/fields/1/last.txt
//
void GetCloud() {
  int iBill;

  Serial.println("GetCloud Start");
  for (iBill=2;iBill<=3;iBill++) {
    if (client.connect(server,80)) {
      String postStr = "GET /channels/" + String(billChannel[iBill]) + "/fields/1/last.txt HTTP/1.1\n";      
      client.print(postStr);
      client.print("Host: api.thingspeak.com\n");
      client.print("User-Agent: ESP8266\n");
      client.print("Connection: close\n");
      client.print("\n\n");

      while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
          break;
        }
      }

      String linenull = client.readStringUntil('\n');
      String line = client.readStringUntil('\n');
      long iPPM = line.toInt();
      (iPPM > POLLUTION_LIMIT) ? digitalWrite(led_Bills[iBill], HIGH) : digitalWrite(led_Bills[iBill], LOW);
      client.stop();          
    } 
    delay(100);
  }
  Serial.println("GetCloud Stop");
}


//
// Arduino SETUP function
//
void setup() {
  int iLed;
  
  // Init Serial
  Serial.begin(115200);
  Serial.println("Start");

  // Init LEDs
  for (iLed=2;iLed<=NB_BILLS;iLed++) {
    pinMode(led_Bills[iLed], OUTPUT);
    digitalWrite(led_Bills[iLed], LOW);  
  }

  // Init Display
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  // Connect to Wifi
  ConnectWifi();

  // Connect to BMP
  ConnectBMP280();

  // Set Presence Sensor GPIO mode
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);
}

//
// Arduino LOOP function
//
void loop() {
  // Clear Display
  display.clear();

  // Display Wifi Info
  display.drawString(0,0, ssid );
  display.drawString(0,10, WiFi.localIP().toString().c_str());

  // Get & Display PPM
  sensorValue = analogRead(A0);
  Serial.println("PPM = "+String(sensorValue));
  display.drawString(0,20, String(sensorValue) + " PPM");

  // Get & Display Temperatture & Altitude
  myTemperature = bme.readTemperature();
  myAltitude = bme.readAltitude(1013.25);
  Serial.println("Temp = "+String(myTemperature));
  Serial.println("Altitude = "+String(myAltitude));
  display.drawString(0,30, String(myTemperature) + " C");
  display.drawString(0,40, String(myAltitude) + " m");

  // Get & Display Presence
  Active = (GetDistance() < DISTANCE);
  Serial.println("Active = "+String(Active));
  (Active) ? display.drawString(0,DISTANCE, "ACTIVE") : display.drawString(0,DISTANCE, "INACTIVE");

  // Flush Display
  display.display();

  // Post & Get Data
  PostCloud();  // Send Data for current BillBoard
  GetCloud();   // Read Data from other BillBoards

  // Wait
  delay(1000);
}
