/*
  Program Title: Select Button Polling Example
  Author: Andreas Kucher
  Date: April 29, 2025

  Program Summary:
  This program demonstrates how to initialize the select button
  on K-COMP and detect whether it is currently pressed using
  a polling approach (not callbacks).

  It uses kcomp_button_select.h.
  A message is printed to the Serial Monitor whenever the
  select button is detected as pressed.
*/

// Include the header file that contains functions for initializing
// and reading the state of the select button
#include "kcomp_button_select.h"

// The setup() function runs once when K-COMP is powered on or reset
void setup() {
  // Start serial communication at 115200 baud
  Serial.begin(115200);

  // Initialize the select button hardware and internal state
  initButtonSelect();
  Serial.println("Select button initialized...");
}

// The loop() function runs repeatedly after setup()
// This version simply checks whether the button is pressed
// and prints a message if it is
void loop() {
  // Read the current button state (true if pressed, false otherwise)
  bool isPressed = isButtonSelectPressed();
  
  // If the button is pressed, print a message
  if (isPressed) {
    Serial.println("Select button is pressed!");
  }

  // Wait 200 ms before checking again (limits output rate)
  delay(200);
}