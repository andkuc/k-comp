/*
  Program Title: ESP8266 Baud Rate Configuration
  Author: Andreas Kucher
  Date: January 11, 2026

  Program Summary:
  This program runs a utility to configure the ESP8266 WiFi module to 9600 baud.
  New ESP8266 modules often default to 115200 baud, which is too fast for
  reliable software communication on the K-Comp.

  USAGE:
  1. Upload this sketch once.
  2. Open Serial Monitor (115200 baud).
  3. Wait for the "Success" confirmation.
  4. You can then upload other Wi-Fi examples.
*/

#include "kcomp_esp8266.h" // Include the new wrapper library

// Define the pins where the ESP8266 is connected
// RX Pin (K-Comp receives data here) -> Connects to ESP TX
const int ESP_RX_PIN = 7;
// TX Pin (K-Comp transmits data from here) -> Connects to ESP RX (via voltage divider)
const int ESP_TX_PIN = 6;

void setup() {
  Serial.begin(115200);         // Start the Serial Monitor at 115200 baud
  
  // Wait a moment for the Serial Monitor to initialize so the user sees the start message
  delay(2000); 

  Serial.println("ESP8266 Baud Rate Tool initialized...");
  Serial.println("Attempting to configure ESP8266 to 9600 baud...");
  Serial.println("PLEASE WAIT - Do not disconnect power.");

  // Call the independent configuration function from the library.
  // This function automatically tries common speeds (115200 and 9600)
  // and sends the AT command to permanently save the new setting.
  setupESP8266BaudRate(ESP_RX_PIN, ESP_TX_PIN);

  Serial.println("---------------------------------------------");
  Serial.println("Configuration sequence complete.");
  Serial.println("Check the output above for 'Success'.");
  Serial.println("If successful, your module is ready for use.");
  Serial.println("---------------------------------------------");
}

void loop() {
  // Since configuration is a one-time action, we have nothing to do in the loop.
  // We just delay to keep the processor idle.
  delay(1000);
}