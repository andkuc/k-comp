/*
  Program Title: Simple Joystick Demo
  Author: Andreas Kucher
  Date: April 29, 2025

  Program Summary:
  This program demonstrates how to use the joystick module of KCOMP.
  The program continuously reads the joystick's X and Y axis values, 
  determines the direction and intensity from the values, and prints 
  this information to the Serial Monitor.
*/

#include <kcomp_joystick.h> // Include the joystick library

// A string array that maps enum values to human-readable direction names
const char* directionNames[] = {
  "CENTER",       // 0
  "LEFT",         // 1
  "RIGHT",        // 2
  "UP",           // 3
  "DOWN",         // 4
  "UP_LEFT",      // 5
  "UP_RIGHT",     // 6
  "DOWN_LEFT",    // 7
  "DOWN_RIGHT",   // 8
  "UNDEFINED"     // 9
};

void setup() {
  Serial.begin(115200);         // Start the Serial Monitor at 115200 baud (bits per second)

  initJoystick();               // Initialize the joystick pins as inputs (X on A2, Y on A1)
  
  Serial.println("Joystick demo initialized...");
  delay(1000);                  // Wait 1 second before starting
}

void loop() {
  // getJoystickXRaw() reads the raw values from the joystick
  // (values from 0 (minimum) to 1023 (maximum)).
  // The rest position of the joystick should be around 512.
  // getJoystickXValue() reads the adjusted raw values (from -512 to 511).
  Serial.print("Raw X: ");
  Serial.print(getJoystickXRaw());
  Serial.print(" | Centered X: ");
  Serial.print(getJoystickXValue()); // Adjusted to -512 to +511

  Serial.print(" | Raw Y: ");
  Serial.print(getJoystickYRaw());
  Serial.print(" | Centered Y: ");
  Serial.print(getJoystickYValue()); // Adjusted to -512 to +511

  // Get the current state of the joystick:
  // - direction: one of the 9 enum values defined in joystickDirection
  // - intensity: 0–100 (%), or -1 if direction is undefined
  joystickState state = getJoystickState();

  // Print the interpreted direction (enum as a readable string)
  Serial.print(" || Direction: ");
  Serial.print(directionNames[state.direction]);

  // Print intensity in percent (0–100%)
  // Intensity is based on how far the joystick is moved from the center
  Serial.print(" | Intensity: ");
  Serial.print(state.intensity);
  Serial.println("%");

  delay(250); // Wait 250 milliseconds before next reading to reduce Serial output frequency
}