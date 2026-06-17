/*
  Program Title: Bluetooth Basic IO
  Author: Andreas Kucher
  Date: July 31, 2025

  Program Summary:
  This program demonstrates how read and write text using the 
  HC-05 bluetooth module. 
  Any message which is received via bluetooth is fowarded to USB-serial.
  Any message which is received via USB-serial is fowarded to bluetooth-serial.
*/

// Include the SoftwareSerial library
// This library allows you to use other digital pins for serial communication
#include "SoftwareSerial.h"

// Create a software serial object named BTSerial
// Pin 7 will be used to receive data (RX)
// Pin 6 will be used to send data (TX)
SoftwareSerial BTSerial(7, 6);

void setup() {
  // Start the standard serial communication with the computer
  // This lets you use the Serial Monitor in the Arduino IDE
  Serial.begin(9600);   

  // Start the software serial communication with the Bluetooth module (HC-05)
  BTSerial.begin(9600); 
}

void loop() {
  // Check if the Bluetooth module has sent any data
  // If yes, read the data and forward it to the Serial Monitor (computer)
  if (BTSerial.available()) {
    // Read one byte from Bluetooth and send it to the Serial Monitor
    Serial.write(BTSerial.read());
  }

  // Check if the computer (Serial Monitor) has sent any data
  // If yes, read the data and forward it to the Bluetooth module
  if (Serial.available()) {
    // Read one byte from the Serial Monitor and send it to Bluetooth
    BTSerial.write(Serial.read());
  }
}
