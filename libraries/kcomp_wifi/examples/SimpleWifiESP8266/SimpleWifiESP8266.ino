/*
  Program Title: HTTPS Client Demo
  Author: Gemini (on behalf of K-Comp Team)
  Date: January 11, 2026

  Program Summary:
  This program demonstrates how to use the ESP8266 wrapper library to:
  1. Connect to a Wi-Fi Access Point.
  2. Display connection details (IP Address, Signal Strength).
  3. Connect to a secure HTTPS server (SSL/TLS).
  4. Perform a GET request to fetch data from a website.
  
  Hardware Requirements:
  - K-Comp Board
  - ESP8266 connected to Pins 2 (RX) and 3 (TX)
  - NOTE: ESP8266 must be set to 9600 baud (use the Baud Rate tool first).
*/

#include "kcomp_ESP8266.h" // Include the ESP8266 wrapper library

// --- Configuration Constants ---
const int ESP_RX_PIN = 7; // Connect to ESP TX
const int ESP_TX_PIN = 6; // Connect to ESP RX

// WiFi Credentials - Change these to match your network
const char* WIFI_SSID = "your_ssid";
const char* WIFI_PASS = "your_password";

// Target Server Settings
// We will use httpbin.org, a service that replies with the data we sent.
const char* SERVER_DOMAIN = "google.com"; 
const int   SERVER_PORT   = 80; // Port 443 indicates HTTPS (SSL)

void setup() {
  Serial.begin(115200);         // Start Serial Monitor for debugging
  delay(2000);                  // Wait for monitor to open

  Serial.println("Initializing ESP8266 Demo...");

  // Initialize the ESP8266 module and connect to the Access Point.
  // This function handles the "AT+CWJAP" command internally.
  // It returns 'true' only if the connection is successful.
  if (beginESP8266(ESP_RX_PIN, ESP_TX_PIN, WIFI_SSID, WIFI_PASS)) {
      Serial.println("WiFi Connected");
      
      // 1. SYNC TIME (Essential for SSL)
      // Use 1 for Austria Winter, 2 for Austria Summer
      syncESP8266Time(1); 
      
      // 2. SET BUFFER
      setSSLBufferSize(8192); // Try 8192 first, then 16384
  } else {
    Serial.println("WiFi Connection Failed. Check SSID/Password or Power.");
    while(1); // Stop here if we can't connect
  }

  // Print the IP Address and Signal Strength to verify status
  printESP8266WifiStatus();
}

void loop() {
  Serial.println("\n--- Starting HTTPS Request ---");

  // Attempt to connect to the server.
  // The library automatically detects Port 443 and uses "SSL" mode.
  if (connectESP8266Server(SERVER_DOMAIN, SERVER_PORT)) {
    Serial.println("Connected to Server!");

    // Send the HTTP GET request.
    // We must send the "Host" header so the server knows who we are.
    // "Connection: close" tells the server to disconnect after sending data.
    printlnESP8266("GET /get HTTP/1.1");
    printlnESP8266("Host: httpbin.org");
    printlnESP8266("Connection: close");
    printlnESP8266(""); // Empty line indicates end of headers

    // Read the response loop
    // We wait for data to arrive and print it to the Serial Monitor
    // as it streams in (byte by byte).
    unsigned long timeout = millis();
    while (millis() - timeout < 5000) { // 5-second timeout safeguard
      
      // Check if data is available in the buffer
      while (availableESP8266()) {
        char c = readESP8266();
        Serial.write(c); // Print character to local monitor
        timeout = millis(); // Reset timeout since we got data
      }
    }
    
    Serial.println("\n--- End of Response ---");
    
    // Cleanly close the connection
    stopESP8266();
    
  } else {
    Serial.println("Connection to Server Failed.");
  }

  // Wait 10 seconds before fetching data again to be polite to the server
  delay(10000);
}