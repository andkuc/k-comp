#include "kcomp_TOF400C.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("K-Comp Laser LIDAR System");
  Serial.println("-------------------------");
  
  // 1. Initialize the sensor hardware
  if (!initTOF400C()) {
    Serial.println("Error: TOF400C not found. Check I2C wiring!");
    while(1); // Stop here and halt the board
  }
  
  Serial.println("Laser Online!");
  
  // Optional: Set to short range if you are in a very bright room near a window
  // setTOF400CMode(TOF400C_MODE_SHORT); 
}

void loop() {
  // 2. Read the sensor continuously
  // The function automatically returns instantly if the laser is still bouncing
  tof400c_data_t laser = readTOF400C();
  
  // 3. Only print if a new pulse has successfully returned
  if (laser.is_valid) {
    
    // Convert to centimeters for easier reading
    float distance_cm = laser.distance_mm / 10.0;
    
    Serial.print("Target Distance: ");
    Serial.print(laser.distance_mm);
    Serial.print(" mm  |  ");
    Serial.print(distance_cm);
    Serial.println(" cm");
  } 
  delay(500);
  // Notice there is no delay() here! 
  // Because readTOF400C() checks the hardware flag, the loop can run as fast
  // as possible without locking up the Arduino.
}