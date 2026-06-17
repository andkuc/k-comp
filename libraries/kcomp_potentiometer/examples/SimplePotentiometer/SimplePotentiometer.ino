/*
  Program Title: Simple Potentiometer
  Author: Andreas Kucher
  Date: April 29, 2025

  Program Summary:
  This program demonstrates how to initialize the potentiometer
  on K-COMP, and continuously read thevalue of the potentiometer. 
  It uses kcomp_potentiometer.h.
  Serial messages are printed to the Serial Monitor.
*/

// Include the header file that contains functions related to the
// potentiometer.
#include <kcomp_potentiometer.h>

// The setup() function runs once when KCOMP is powered on or reset
void setup() {
  // Start serial communication at 115200 baud for messages
  Serial.begin(115200);

  // Initialize the potentiometer
  initPotentiometer();
  // Print a message to the Serial Monitor
  Serial.println("Potentiometer initialized...");
}

// The loop() function runs over and over again forever
void loop() {
  Serial.println("Reading the raw value (0-1023) of the potentiometer: ");
  int raw = getPotentiometerRaw();
  Serial.print("Raw value: ");
  Serial.println(raw);

  Serial.println("Reading the scaled value (0-100) of the potentiometer: ");
  int value = getPotentiometerValue();
  Serial.print("Scaled value: ");
  Serial.println(value);
  
  // Wait 1s
  delay(1000);
}