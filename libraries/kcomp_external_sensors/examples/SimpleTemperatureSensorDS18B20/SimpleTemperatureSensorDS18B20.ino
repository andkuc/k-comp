#include "kcomp_DS18B20.h"

// We are connecting the Yellow Data wire to Digital Pin 12
#define TEMP_PIN 12

void setup() {
  Serial.begin(115200);
  
  Serial.println("K-Comp Thermometer Online");
  
  if (!initDS18B20(TEMP_PIN)) {
    Serial.println("Error: DS18B20 not found! Did you forget the 4.7k resistor?");
    while(1); 
  }
  
  // Ask the sensor to start measuring before we enter the loop
  requestDS18B20(); 
}

void loop() {
  
  // Check if the sensor has finished its 750ms calculation
  ds18b20_data_t temp_sensor = readDS18B20();
  
  if (temp_sensor.is_valid) {
    
    Serial.print("Temperature: ");
    Serial.print(temp_sensor.temperature_c);
    Serial.println(" °C");
    
    // Immediately ask it to start the NEXT measurement
    requestDS18B20();
  }
}