/*
  Program Title: Simple Distance Measurement
  Author: Andreas Kucher
  Date: April 29, 2025

  Program Summary:
  This program demonstrates how to initialize the distance sensor
  on K-COMP, and continuously read the distance measured by the 
  sensor. It uses kcomp_distance.h.
  Serial messages are printed to the Serial Monitor.
*/

// Include the header file that contains functions related to the
// distance sensor HC-SR04.
#include <kcomp_distance.h>

// The setup() function runs once when KCOMP is powered on or reset
void setup() {
  // Start serial communication at 115200 baud for messages
  Serial.begin(115200);

  // Initialize the distance sensor
  initDistanceSensor();
  // Print a message to the Serial Monitor
  Serial.println("Distance Sensor initialized...");
}

// The loop() function runs over and over again forever
void loop() {
  Serial.println("Reading the current distance (works for distances from 2cm to 4m).");
  
  // Read the distance value from the sensor
  int distance = getDistanceMM();
  // Print the distance to the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println("mm");
 
  // Wait 1s
  delay(1000);
}