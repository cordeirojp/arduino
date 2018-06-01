//
// BillBoard Arduino ESP8266 Code
// (C) ESP Porto Team #3 2018
//
// Code for Unit Test BME.01 : Acquire Temperature
// Code for Unit Test BME.02 : Rightness of Temperature
//

// Libraries for I2C and OLED Display
#include <Wire.h>
#include "SSD1306Wire.h"

// Libraries for Temperature Sensor
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// Global Variables
float myTemperature;                  // Temperature
SSD1306Wire display(0x3c, 4, 5);    // OLED Object (Address, SDA Pin, SDL Pin)
Adafruit_BMP280 bme;                // BMP Object

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
// Arduino SETUP function
//
void setup() {  
  // Init Serial
  Serial.begin(115200);
  Serial.println("Start");

  // Init Display
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  // Connect to BMP
  ConnectBMP280();
}

//
// Arduino LOOP function
//
void loop() {
  // Clear Display
  display.clear();

  // Get & Display Temperature
  myTemperature = bme.readTemperature();
  Serial.println("Temp = "+String(myTemperature));
  display.drawString(0,30, String(myTemperature) + " C");

  // Flush Display
  display.display();

  // Wait
  delay(1000);
}
