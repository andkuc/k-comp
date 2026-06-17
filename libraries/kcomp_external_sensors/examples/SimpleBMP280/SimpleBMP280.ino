/*
 * kcomp_BMP280_Example.ino
 *
 * Example sketch for using the simplified BMP280 library.
 * Reads temperature, pressure, and approximate altitude,
 * then prints them to the Serial Monitor.
 */

#include "kcomp_BMP280.h"

void setup() {
  // 1. Initialize Serial for printing results
  Serial.begin(115200); // Standard baud rate
  
  Serial.println("BMP280 Sensor Test");

  // 2. Initialize the BMP280 sensor
  // This starts I2C, checks connection, and sets up calibration data.
  if (!initBMP280()) {
    Serial.println("Error: BMP280 not found! Check wiring.");
    while (1); // Stop here if sensor is missing
  }
  
  Serial.println("BMP280 Initialized.");
}

void loop() {
  // 3. Read values using the simple library functions
  float temperature = getBMP280Temperature();
  float pressurePa = getBMP280Pressure();       // Returns Pascals
  float altitude = getBMP280Altitude(1013.25);  // Estimated altitude (Standard Sea Level)

  // 4. Print results to Serial Monitor
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" C\t");
  
  Serial.print("Press: ");
  Serial.print(pressurePa / 100.0); // Convert Pa to hPa (hectopascals)
  Serial.print(" hPa\t");

  Serial.print("Alt: ");
  Serial.print(altitude);
  Serial.println(" m");

  // Wait 1 second before the next reading
  delay(1000);
}