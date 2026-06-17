/*
  kcomp_ESP8266.h - Flat C-style wrapper for ESP8266 AT Commands.
  Designed for K-Comp (Arduino Uno compatible) using SoftwareSerial.
*/

#ifndef _KCOMP_ESP8266_H
#define _KCOMP_ESP8266_H

#include "Arduino.h"

// --- Configuration ---
#define ESP_TIMEOUT_DEFAULT 5000 

// --- Core Functions ---

/*
 * Independent utility to force the ESP8266 to 9600 baud.
 */
void setupESP8266BaudRate(int rxPin, int txPin);

/*
 * Initialize the module and connect to WiFi.
 * Returns true if connected successfully.
 */
bool beginESP8266(int rxPin, int txPin, const char* ssid, const char* pass);

/*
 * Sync time with internet servers (Required for SSL/HTTPS).
 * timezone: 1 for CET (Winter), 2 for CEST (Summer).
 * Returns true if year is > 2020.
 */
bool syncESP8266Time(int timezone);

/*
 * Set the SSL buffer size.
 * Values: 4096 (standard), 8192 (Google), 16384 (Max).
 * Call this in setup() AFTER beginESP8266().
 */
bool setSSLBufferSize(uint16_t size);

/*
 * Connect to a server (TCP or SSL).
 * Automatically selects "SSL" mode if port is 443.
 * Returns true if connection established.
 */
bool connectESP8266Server(const char* domain, int port);

/*
 * Send text data to the connected server followed by a newline.
 */
void printlnESP8266(const char* data);

/*
 * Read/Check data.
 */
int readESP8266(void);
int availableESP8266(void);
void printESP8266WifiStatus(void);
void stopESP8266(void);

#endif