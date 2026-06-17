#include "kcomp_MQSensors.h"
// ====================================================================
// SELECT YOUR SENSOR HERE
// SENSOR_MQ2   (Flammable Gas & Smoke)
// SENSOR_MQ3   (Alcohol Vapor)
// SENSOR_MQ135 (Air Quality & VOCs)
// ====================================================================

const uint8_t ACTIVE_SENSOR = SENSOR_MQ3; 

// ====================================================================

#define MQ_PIN A3  

void setup() {
  Serial.begin(115200);
  
  initMQ(MQ_PIN, ACTIVE_SENSOR);
  
  Serial.print("Initializing Sensor: ");
  Serial.println(ACTIVE_SENSOR);
  
  Serial.println("Warming up heater... (Keep in fresh air!)");
  delay(3000); 
  
  Serial.println("Calibrating baseline...");
  calibrateMQ(MQ_PIN);
  Serial.println("Calibration complete. Sensor is live!\n");
}

void loop() {
  // 1. Get the Universal 0-100% Score
  uint8_t intensity = readSensorScore(MQ_PIN);
  
  // 2. Get the Universal Status Category
  uint8_t status = readSensorStatus(MQ_PIN);
  
  // 3. Print the Label (Optional formatting so the serial monitor looks nice)
  if (ACTIVE_SENSOR == SENSOR_MQ2)        Serial.print("[MQ-2 Smoke]   Intensity: ");
  else if (ACTIVE_SENSOR == SENSOR_MQ3)   Serial.print("[MQ-3 Alcohol] Intensity: ");
  else if (ACTIVE_SENSOR == SENSOR_MQ135) Serial.print("[MQ-135 VOCs]  Intensity: ");
  
  // 4. Print the Math
  Serial.print(intensity);
  Serial.print("%  |  Status: ");
  
  // 5. Print the Warning Level
  if (status == STATUS_CLEAN) {
      Serial.println("CLEAN");
  } else if (status == STATUS_MODERATE) {
      Serial.println("MODERATE");
  } else if (status == STATUS_HIGH) {
      Serial.println("HIGH WARNING!");
  }
  
  delay(1000);
}