/*
  Program Title: LED Control (with Fading)
  Author: Andreas Kucher
  Date: April 29, 2025

  Program Summary:
  This program demonstrates how to initialize the LED on K-COMP,
  turn it on with different brightness levels, and perform fade-in
  and fade-out animations. It uses kcomp_led.h.
  Serial messages are printed to the Serial Monitor.
*/

// Include the header file that contains LED control functions
#include <kcomp_led.h>

// The setup() function runs once when KCOMP is powered on or reset
void setup() {
  // Start serial communication at 115200 baud for messages
  Serial.begin(115200);

  // Initialize the LED pin for output
  initLed();
  // Print a message to the Serial Monitor
  Serial.println("LED initialized...");

  // Turn the LED on at 50% brightness
  ledOn(50);
  // Inform the user
  Serial.println("LED switched on at 50% for 1s.");
  // Wait for 1 second
  delay(1000);

  // Increase LED brightness to  100%
  ledOn(100);
  Serial.println("LED switched on at 100% for 1s.");
  // Wait for 1 second
  delay(1000);

  // Turn the LED completely off
  ledOff();
  Serial.println("LED switched off.");
  // Wait for 1 second before starting loop()
  delay(1000);
}

// The loop() function runs over and over again forever
void loop() {
  Serial.println("Fading in LED from 0% to 100% within 1s (blocking!).");
  // Slowly turn the LED on to 100 brightness in 1 second
  ledFadeIn(100, 1000);
  
  Serial.println("Fading out LED from 100% to 0 within 1s (blocking!).");
  // Slowly turn the LED off from 100 brightness in 1 second
  ledFadeOut(100, 1000);
}

// INFO:
// "Blocking" means that the Processor will not do anything else while a 
// blocking function runs. For example, during ledFadeIn() or ledFadeOut(), 
// the Processor waits (delays) to complete the fade effect and can't react 
// to inputs, buttons, or run other code at the same time.