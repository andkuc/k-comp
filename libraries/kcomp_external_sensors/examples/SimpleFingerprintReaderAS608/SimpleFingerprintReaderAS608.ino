#include "kcomp_AS608.h"

// --- Wiring Definitions ---
// K-Comp 3.3V Pin 6 (RX) goes to Sensor TX
// K-Comp 3.3V Pin 7 (TX) goes to Sensor RX
#define RX_PIN 6
#define TX_PIN 7

#define BAUD_RATE 57600

// --- Helper Variables ---
bool menuPrinted = false;

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for Serial Monitor to open

  Serial.println("\n==================================");
  Serial.println("  AS608 MASTER CONTROL CONSOLE    ");
  Serial.println("==================================");
  
  Serial.println("Connecting to Sensor...");
  
  if (initAS608(RX_PIN, TX_PIN, BAUD_RATE)) {
    Serial.println("[SUCCESS] Sensor Found and Ready!");
    
    // Optional: Flash the LED Blue to show it's alive (if it's an R503 model)
    controlLED(LED_FLASHING, LED_BLUE, 100, 2);
  } else {
    Serial.println("[ERROR] Sensor not found.");
    Serial.println("1. Check your wiring (TX-> 3.3V RX, RX-> 3.3V TX).");
    Serial.println("2. Ensure VCC is plugged into 3.3V!).");
    Serial.println("2. Ensure VCC is plugged into 3.3V!).");
    while (1); // Halt the program completely
  }
}

void loop() {
  // Print the menu only once until an action is completed
  if (!menuPrinted) {
    printMenu();
    menuPrinted = true;
  }

  // Wait for the user to type something
  if (Serial.available() > 0) {
    char choice = Serial.read();
    
    // Clean up any lingering invisible characters like 'Enter' (\n or \r)
    clearSerialBuffer(); 
    
    // Process the user's choice
    switch (choice) {
      
      case '1': {
        Serial.println("\n>>> [MODE: SCAN FINGERPRINT]");
        Serial.println("Place your finger on the glass...");
        
        // Flash purple while waiting
        controlLED(LED_BREATHING, LED_PURPLE, 100, 0); 
        
        int16_t id = -1;
        // Loop until a finger is found or an error occurs
        while (id == -1) {
            id = scanFingerprint();
        }
        
        if (id >= 0) {
            Serial.print("[GRANTED] Welcome! You are ID #");
            Serial.println(id);
            controlLED(LED_ALWAYS_ON, LED_BLUE, 100, 0); // Success light
        } else {
            Serial.println("[DENIED] Unknown Fingerprint.");
            controlLED(LED_FLASHING, LED_RED, 100, 2); // Error light
        }
        delay(2000); // Give them time to read it
        break;
      }
      
      case '2': {
        Serial.println("\n>>> [MODE: ENROLL NEW USER]");
        Serial.print("Enter an ID number (1 to 127) to save this finger to: ");
        
        int targetID = getNumberFromSerial();
        Serial.println(targetID); // Echo their choice
        
        // Prevent accidental overwrites
        if (isSlotOccupied(targetID)) {
            Serial.println("[WARNING] That ID is already taken! Try another one.");
        } else {
            int8_t status = enrollFingerprint(targetID);
            if (status == AS608_OK) {
                controlLED(LED_FLASHING, LED_BLUE, 100, 3);
            } else {
                controlLED(LED_FLASHING, LED_RED, 100, 3);
            }
        }
        delay(2000);
        break;
      }
      
      case '3': {
        Serial.println("\n>>> [MODE: CHECK DATABASE CAPACITY]");
        int count = getFingerprintCount();
        if (count >= 0) {
            Serial.print("There are currently [ ");
            Serial.print(count);
            Serial.println(" ] fingerprints stored in the sensor.");
        } else {
            Serial.println("[ERROR] Could not read database.");
        }
        delay(2000);
        break;
      }
      
      case '4': {
        Serial.println("\n>>> [MODE: DELETE SINGLE USER]");
        Serial.print("Enter the ID number to delete: ");
        int targetID = getNumberFromSerial();
        Serial.println(targetID);
        
        if (deleteFingerprint(targetID) == AS608_OK) {
            Serial.println("[SUCCESS] Fingerprint deleted permanently.");
        } else {
            Serial.println("[ERROR] Could not delete. Maybe the slot was already empty?");
        }
        delay(2000);
        break;
      }
      
      case '5': {
        Serial.println("\n>>> [MODE: FACTORY RESET]");
        Serial.println("Are you sure you want to delete EVERY fingerprint? (y/n)");
        
        // Wait for confirmation
        while (!Serial.available());
        char confirm = Serial.read();
        clearSerialBuffer();
        
        if (confirm == 'y' || confirm == 'Y') {
            if (clearAllFingerprints()) {
                Serial.println("[DELETED] The database is now empty.");
                controlLED(LED_FLASHING, LED_RED, 50, 5);
            } else {
                Serial.println("[ERROR] Failed to wipe database.");
            }
        } else {
            Serial.println("Factory Reset Cancelled.");
        }
        delay(2000);
        break;
      }
      
      default:
        // Ignore random characters and enters
        break;
    }
    
    // Ask the loop to print the menu again on the next pass
    menuPrinted = false; 
    
    // Turn the LED off while waiting in the menu
    controlLED(LED_ALWAYS_OFF, LED_BLUE, 0, 0); 
  }
}

// ==========================================================
// UI HELPER FUNCTIONS
// ==========================================================

void printMenu() {
  Serial.println("\n----------------------------------");
  Serial.println("MAIN MENU - Type a number and press Enter:");
  Serial.println("  1. Scan Fingerprint");
  Serial.println("  2. Enroll New Fingerprint");
  Serial.println("  3. Check Total Enrolled Count");
  Serial.println("  4. Delete a Specific ID");
  Serial.println("  5. ERASE ENTIRE DATABASE");
  Serial.println("----------------------------------");
  Serial.print("Your choice: ");
}

// Safely grabs a typed number from the serial monitor
int getNumberFromSerial() {
  while (!Serial.available()) {
      // Do nothing, just wait for the user to type the number
  }
  int num = Serial.parseInt(); 
  clearSerialBuffer(); // Clean up the enter key they pressed
  return num;
}

// Empties out the serial pipeline to prevent glitches
void clearSerialBuffer() {
  delay(10); // Give the data a microsecond to arrive
  while (Serial.available()) {
    Serial.read();
  }
}