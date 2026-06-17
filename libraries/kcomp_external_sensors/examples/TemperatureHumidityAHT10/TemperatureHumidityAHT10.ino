/*
 * kcomp_AHT10_Example.ino
 *
 * Example sketch for using the simplified AHT10 library.
 * Reads temperature and humidity and prints them to the Serial Monitor.
 */

#include "kcomp_AHT10.h"

void setup() {
  // 1. Initialize Serial for printing results
  Serial.begin(115000);
  while (!Serial) {
    ; // Wait for serial port to connect (needed for some boards)
  }
  
  Serial.println("AHT10 Sensor Test");

  // 2. Initialize the AHT10 sensor
  // This starts the Wire library and sends the calibration command
  initAHT10();
  
  Serial.println("AHT10 Initialized.");
}

void loop() {
  // 3. Read values using the simple library functions
  float temperature = getAHT10Temperature();
  float humidity = getAHT10Humidity();

  // 4. Print results to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" C\t");
  
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Wait 1 second before the next reading
  delay(1000);
}