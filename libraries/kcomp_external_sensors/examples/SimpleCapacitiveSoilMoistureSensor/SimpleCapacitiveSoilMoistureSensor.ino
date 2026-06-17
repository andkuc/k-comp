#include "kcomp_SoilSensor.h"

void setup() {
  Serial.begin(115200);
  
  // Initialize the sensor on Analog Pin 0
  initSoilSensor(A3);
  
  // Custom calibration for this specific sensor
  // Measure yourself if you need higher precision!
  // Values: Dry Air = 560, Glass of Water = 220
  setSoilCalibration(560, 220);
  
  Serial.println("K-Comp Soil Monitor Online");
}

void loop() {
  // Grab the values using our two clean functions
  uint16_t raw_val = readSoilRaw();
  uint8_t moisture = readSoilMoisture();
  
  Serial.print("Raw Analog: ");
  Serial.print(raw_val);
  Serial.print("  |  Moisture: ");
  Serial.print(moisture);
  Serial.println("%");
  
  delay(1000); 
}