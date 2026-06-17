/*
  Program Title: Start Button Callback
  Author: Andreas Kucher
  Date: April 29, 2025

  Program Summary:
  This program demonstrates how to initialize the start button
  on K-COMP and handle various button events:
  - single click
  - double click
  - long press
  - long press release

  It uses kcomp_button_start.h.
  Serial messages are printed to the Serial Monitor to indicate button events.
*/

// Include the header file that contains functions for handling
// the start button and its event detection (click, long press, etc.)
#include "kcomp_button_start.h"

// --- Event Callback Functions ---

// This function will be called when the button is clicked once
void onButtonStartClick() {
  Serial.println("Start button clicked");
}

// This function will be called when the button is double-clicked
void onButtonStartDoubleClick() {
  Serial.println("Start button double-clicked");
}

// This function will be called when the button is long pressed
void onButtonStartLongPress() {
  Serial.println("Start button longpress");
}

// This function will be called when a long press is released
void onButtonStartLongRelease() {
  Serial.println("Start button longpress released");
}

// The setup() function runs once when K-COMP is powered on or reset
void setup() {
  // Start serial communication at 115200 baud
  Serial.begin(115200);

  // Initialize the start button hardware and internal state
  initButtonStart();
  Serial.println("Start button initialized...");

  // Register callback functions to handle specific button events:
  // Each function will be called automatically when the event occurs
  attachButtonStartClick(onButtonStartClick);                   // Single click
  attachButtonStartDoubleClick(onButtonStartDoubleClick);       // Double click
  attachButtonStartLongPressed(onButtonStartLongPress);         // Long press detected
  attachButtonStartLongReleased(onButtonStartLongRelease);      // Long press released
}

// The loop() function runs repeatedly after setup()
// It's used here to continuously poll the button state
// to detect clicks and long presses
void loop() {
  pollStartButton(); // Checks if the button was pressed, held, or released
  delay(50);         // Small delay to limit polling frequency (debouncing)
}