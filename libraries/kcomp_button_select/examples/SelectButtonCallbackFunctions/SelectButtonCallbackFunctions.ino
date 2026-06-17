/*
  Program Title: Select Button Callback
  Author: Andreas Kucher
  Date: April 29, 2025

  Program Summary:
  This program demonstrates how to initialize the select button
  on K-COMP and handle various button events:
  - single click
  - double click
  - long press
  - long press release

  It uses kcomp_button_select.h.
  Serial messages are printed to the Serial Monitor to indicate button events.
*/

// Include the header file that contains functions for handling
// the select button and its event detection (click, long press, etc.)
#include "kcomp_button_select.h"

// --- Event Callback Functions ---

// This function will be called when the button is clicked once
void onButtonSelectClick() {
  Serial.println("Select button clicked");
}

// This function will be called when the button is double-clicked
void onButtonSelectDoubleClick() {
  Serial.println("Select button double-clicked");
}

// This function will be called when the button is long pressed
void onButtonSelectLongPress() {
  Serial.println("Select button longpress");
}

// This function will be called when a long press is released
void onButtonSelectLongRelease() {
  Serial.println("Select button longpress released");
}

// The setup() function runs once when K-COMP is powered on or reset
void setup() {
  // Start serial communication at 115200 baud
  Serial.begin(115200);

  // Initialize the select button hardware and internal state
  initButtonSelect();
  Serial.println("Select button initialized...");

  // Register callback functions to handle specific button events:
  // Each function will be called automatically when the event occurs
  attachButtonSelectClick(onButtonSelectClick);                   // Single click
  attachButtonSelectDoubleClick(onButtonSelectDoubleClick);       // Double click
  attachButtonSelectLongPressed(onButtonSelectLongPress);         // Long press detected
  attachButtonSelectLongReleased(onButtonSelectLongRelease);      // Long press released
}

// The loop() function runs repeatedly after setup()
// It's used here to continuously poll the button state
// to detect clicks and long presses
void loop() {
  pollSelectButton(); // Checks if the button was pressed, held, or released
  delay(50);         // Small delay to limit polling frequency (debouncing)
}