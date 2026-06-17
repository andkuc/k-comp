/*
  kcomp_GY302.cpp - Implementation
*/

#include "kcomp_GY302.h"
#include <Wire.h>

// --- Internal Constants ---
#define CMD_POWER_ON    0x01
#define CMD_RESET       0x07

// --- Internal State ---
static uint8_t _i2c_address = GY302_ADDR_DEFAULT;
static uint8_t _current_mode = GY302_MODE_NORMAL;

// --- Helper: Send Command ---
static bool sendCommand(uint8_t cmd) {
    Wire.beginTransmission(_i2c_address);
    Wire.write(cmd);
    return (Wire.endTransmission() == 0);
}

// --- Public Functions ---

bool initGY302(uint8_t address) {
    _i2c_address = address;
    Wire.begin();

    // Power on
    if (!sendCommand(CMD_POWER_ON)) return false;
    
    // Reset and set default mode
    sendCommand(CMD_RESET);
    setGY302Mode(GY302_MODE_NORMAL);
    
    return true;
}

float getGY302Lux(void) {
    // Request 2 bytes
    Wire.requestFrom(_i2c_address, (uint8_t)2);
    
    if (Wire.available() == 2) {
        uint8_t highByte = Wire.read();
        uint8_t lowByte = Wire.read();
        uint16_t rawLevel = ((uint16_t)highByte << 8) | lowByte;
        
        // Standard BH1750 conversion factor is 1.2
        float lux = rawLevel / 1.2;
        
        // In High Precision Mode 2 (0x11), the resolution is 0.5 lux.
        // This is often implemented as a doubled sensitivity, meaning we divide by 2.
        if (_current_mode == GY302_MODE_HIGH) {
            lux = lux / 2.0;
        }
        
        return lux;
    }
    return -1.0; 
}

void setGY302Mode(GY302Mode mode) {
    _current_mode = mode;
    sendCommand(mode);
    
    // Wait for the measurement cycle to complete so the next read is valid.
    // Fast mode needs ~16ms, Normal/High modes need ~120ms.
    if (mode == GY302_MODE_FAST) {
        delay(20);
    } else {
        delay(180); 
    }
}