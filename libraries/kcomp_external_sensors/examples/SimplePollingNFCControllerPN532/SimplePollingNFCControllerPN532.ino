#include "kcomp_PN532.h"
#define TARGET_SLOT 4 // Safe user memory page/block

// The global data array. MUST be exactly 16 bytes. 
// If the string is shorter, pad it with spaces to overwrite old data properly.
uint8_t globalDataToWrite[16] = "Hello, ABCDEF!!!";

// Buffer to hold the data we read back from the tag
uint8_t dataBuffer[16]; 

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("==================================");
    Serial.println("      PN532: SIMPLE TERMINAL      ");
    Serial.println("==================================");

    if (!initPN532()) {
        Serial.println("[FATAL] Hardware initialization failed.");
        while (1); // Halt the processor
    }
    
    Serial.println("\n--- MAIN MENU ---");
    Serial.println("[ r ] Read data from tag");
    Serial.println("[ w ] Write global data to tag");
    Serial.print("Awaiting command... \n");
}

void loop() {
    // Only proceed if the user has typed something into the Serial Monitor
    if (Serial.available() > 0) {
        
        char command = Serial.read();
        
        // Purge any invisible carriage returns (\r) or newlines (\n)
        while (Serial.available() > 0) { 
            Serial.read(); 
            delay(2); 
        }

        // ====================================================================
        // READ LOGIC
        // ====================================================================
        if (command == 'r' || command == 'R') {
            Serial.println("\n>> READ MODE: Place the tag on the sensor...");
            
            // Trap the program here until a tag physically touches the reader
            while (scanForTag() != PN532_OK) { 
                delay(100); 
            }
            
            Serial.println(">> Tag locked. Extracting payload...");
            
            if (readTagMemory(TARGET_SLOT, dataBuffer) == PN532_OK) {
                Serial.print(">> Data in Slot 4: [ ");
                for (int i = 0; i < 16; i++) {
                    // Only print valid characters to prevent console garbage
                    if (dataBuffer[i] >= 32 && dataBuffer[i] <= 126) {
                        Serial.print((char)dataBuffer[i]);
                    } else {
                        Serial.print(".");
                    }
                }
                Serial.println(" ]");
            } else {
                Serial.println(">> [!] Hardware Read Failed.");
            }
            
            delay(2000); // Give the user time to remove the tag
            Serial.println("\n--- MAIN MENU --- [ r ] Read | [ w ] Write");
        } 
        
        // ====================================================================
        // WRITE LOGIC
        // ====================================================================
        else if (command == 'w' || command == 'W') {
            Serial.println("\n>> WRITE MODE: Place the tag on the sensor...");
            
            // Trap the program here until a tag physically touches the reader
            while (scanForTag() != PN532_OK) { 
                delay(100); 
            }
            
            Serial.println(">> Tag locked. Burning global data to silicon...");
            
            if (writeTagMemory(TARGET_SLOT, globalDataToWrite) == PN532_OK) {
                Serial.println(">> [SUCCESS] Data permanently committed to the tag.");
            } else {
                Serial.println(">> [!] Hardware Write Failed.");
            }
            
            delay(2000); // Give the user time to remove the tag
            Serial.println("\n--- MAIN MENU --- [ r ] Read | [ w ] Write");
        } 
        
        // ====================================================================
        // ERROR HANDLING
        // ====================================================================
        else {
            Serial.println("[!] Invalid command. Type 'r' or 'w'.");
        }
    }
}