/*
 * kcomp_GY302_Example.ino
 * Simple example for reading Light Intensity (Lux).
 */

#include "kcomp_GY302.h"

void setup() {
  Serial.begin(115200);
  Serial.println("GY-302 (BH1750) Light Sensor Test");

  // 1. Initialize Sensor
  if (initGY302(GY302_ADDR_DEFAULT)) {
    Serial.println("Sensor Initialized OK.");
  } else {
    Serial.println("Error: Sensor not found! Check wiring.");
    while(1); // Stop here if failed
  }

  setGY302Mode(GY302_MODE_NORMAL);
  
}

void loop() {
  // 2. Read Lux
  float lux = getGY302Lux();

  if (lux >= 0) {
    Serial.print("Light Level: ");
    Serial.print(lux);
    Serial.println(" lx");

    // Optional: Simple logic for students
    if (lux < 10) {
      Serial.println(" -> Very Dark");
    } else if (lux > 1000) {
      Serial.println(" -> Very Bright");
    }
    
  } else {
    Serial.println("Error reading sensor.");
  }

  delay(500);
}