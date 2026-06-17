#include "kcomp_SPS30.h"

void setup() {
  Serial.begin(9600);
  delay(1000);
  
  Serial.println("K-Comp SPS30 Refactored Monitor");
  
  if (initSPS30()) {
    Serial.println("Sensor warming up...");
  } else {
    Serial.println("Error: SPS30 not found!");
    while(1);
  }
}

void loop() {
  // Grab the whole struct from the sensor
  sps30_data_t air = readSPS30();
  
  // Only print if the struct contains fresh data
  if (air.is_valid) {
    
    // Access the struct directly using a dot!
    Serial.print("\nPM2.5: ");
    Serial.print(air.pm2_5);
    Serial.print(" ug/m3 | PM10: ");
    Serial.print(air.pm10_0);
    Serial.println(" ug/m3");

    // The smart functions still work without parameters
    Serial.print("Clean Score: ");
    Serial.print(getSPS30_AirQualityScore());
    Serial.println(" / 100");

    SPS30_Source_t source = getSPS30_PollutionSource();
    if (source == SPS30_SOURCE_COMBUSTION) {
      Serial.println("⚠️ Warning: Smoke or Exhaust detected!");
    } 
  } 
  
  delay(500); 
}