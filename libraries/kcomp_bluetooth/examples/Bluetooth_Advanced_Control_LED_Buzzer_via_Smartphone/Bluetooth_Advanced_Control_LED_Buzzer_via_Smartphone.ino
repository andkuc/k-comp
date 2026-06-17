/*
  Program Title: Bluetooth Advanced Control (LED - BUZZER via Smartphone)
  Author: Andreas Kucher
  Date: July 31, 2025

  Program Summary:
  This program demonstrates how to implement a very simple text protocol, which 
  allows control of the LED/Buzzer on K-COMP via Bluetooth 
  (e.g. Serial Bluetooth Terminal App on your Android Smartphone).
  Any command you type in the Serial Monitor is sent to K-COMP. 
  If the Start Button is clicked, a message is send to the Smartphone. 
*/

#include "SoftwareSerial.h"          // Allows serial communication on digital pins
#include "kcomp_led.h"               // Custom library to control the LED
#include "kcomp_button_start.h"      // Custom library for the start button
#include "kcomp_buzzer.h"            // Custom library for the buzzer

// Maximum number of characters we accept from a Bluetooth command
#define MAX_INPUT_LENGTH 64

// Create a software serial port on pins 7 (RX) and 6 (TX)
SoftwareSerial BTSerial(7, 6);

// A character array (C-style string) to store incoming text from Bluetooth
char inputBuffer[MAX_INPUT_LENGTH];

// This index tells us how many characters we've received so far
int inputIndex = 0;

// This function is automatically called when the start button is clicked
void onButtonStartClick() {
  BTSerial.println("Start button clicked"); // Send message via Bluetooth
}

void setup() {
  // Start communication with the computer (USB), useful for Serial Monitor
  Serial.begin(9600);

  // Start communication with the Bluetooth module
  BTSerial.begin(9600);

  // Initialize LED, buzzer, and start button
  initLed();
  initBuzzer();
  initButtonStart();

  // Register the function to call when the start button is pressed
  attachButtonStartClick(onButtonStartClick);

  // Send welcome message and help instructions via Bluetooth
  BTSerial.println("K-Comp is ready for your commands.");
  BTSerial.println("You may send: ");
  BTSerial.println("  - LED ON");
  BTSerial.println("  - LED OFF");
  BTSerial.println("  - BEEP [frequency in Hz] [duration in ms]");
}

void loop() {
  // Check the current state of the start button
  pollStartButton();

  // If there is any data waiting from the Bluetooth module
  while (BTSerial.available()) {
    char c = BTSerial.read();  // Read the next character

    // Check if the character is a line break (CR or LF)
    // These characters mean "the user pressed Enter" – end of command
    if (c == '\r' || c == '\n') {
      // Only process if something was typed (non-empty command)
      if (inputIndex > 0) {
        inputBuffer[inputIndex] = '\0'; // Add null character to end the string
        processCommand(inputBuffer);    // Process the full command
        inputIndex = 0;                 // Reset buffer for next command
      }
    }
    // If the character is printable and buffer is not full
    else if (isPrintable(c) && inputIndex < MAX_INPUT_LENGTH - 1) {
      // Convert letter to uppercase so that "led on" or "LED ON" both work
      inputBuffer[inputIndex++] = toupper(c);
    }
  }
}

// This function compares the received command and performs the right action
void processCommand(const char* cmd) {
  // Command: LED ON
  if (strcmp(cmd, "LED ON") == 0) {
    ledOn(100); // Turn LED on with brightness 100
    BTSerial.println("LED was switched on.");
  }

  // Command: LED OFF
  else if (strcmp(cmd, "LED OFF") == 0) {
    ledOff(); // Turn LED off
    BTSerial.println("LED was switched off.");
  }

  // Command starts with "BEEP"
  else if (strncmp(cmd, "BEEP", 4) == 0) {
    int freq = 0, duration = 0;

    // Try to extract two numbers after "BEEP" (e.g. "BEEP 1000 300")
    // sscanf reads numbers from a string and stores them in variables
    if (sscanf(cmd, "BEEP %d %d", &freq, &duration) == 2) {
      buzzerBeepNonBlocking(freq, duration); // Play tone without blocking loop()
      BTSerial.println("Beep command executed!");
    } else {
      // If command format is wrong or missing numbers
      BTSerial.println("Invalid beep command");
    }
  }
  // Unknown command
  else {
    BTSerial.print("Invalid command: ");
    BTSerial.println(cmd);
  }
}