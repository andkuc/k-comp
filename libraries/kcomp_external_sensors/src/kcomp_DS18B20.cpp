#include "kcomp_DS18B20.h"
#include <OneWire.h> // Used strictly for the microsecond timing of the 1-Wire bus

static OneWire* ds_bus = nullptr;
static ds18b20_data_t internalData;
static unsigned long conversionStartTime = 0;
static bool isConverting = false;

// --- DS18B20 Hexadecimal Command Set ---
#define CMD_SKIP_ROM        0xCC  // Targets the only device on the wire
#define CMD_CONVERT_T       0x44  // Instructs the silicon to read the temperature
#define CMD_READ_SCRATCHPAD 0xBE  // Instructs the sensor to transmit its memory

// --- Public Functions ---

bool initDS18B20(uint8_t pin) {
    if (ds_bus != nullptr) {
        delete ds_bus;
    }
    ds_bus = new OneWire(pin);

    // Send a low-level reset pulse. If a sensor is connected, 
    // it will pull the line low in response (Presence Pulse).
    if (ds_bus->reset() == 0) {
        return false; // No device responded
    }
    return true;
}

void requestDS18B20() {
    if (ds_bus == nullptr) return;

    ds_bus->reset();
    ds_bus->write(CMD_SKIP_ROM);   // "Hey you, the only sensor on the line..."
    ds_bus->write(CMD_CONVERT_T);  // "...start measuring the temperature!"
    
    // Record the time we asked it to start
    conversionStartTime = millis();
    isConverting = true;
}

ds18b20_data_t readDS18B20() {
    internalData.is_valid = false;

    if (ds_bus == nullptr || !isConverting) {
        return internalData; 
    }

    // The DS18B20 needs 750ms to complete a high-resolution 12-bit measurement
    if (millis() - conversionStartTime < 750) {
        return internalData; // Still calculating, abort read
    }

    isConverting = false; // The 750ms have passed!

    // Request the memory contents
    ds_bus->reset();
    ds_bus->write(CMD_SKIP_ROM);
    ds_bus->write(CMD_READ_SCRATCHPAD);

    // The sensor sends 9 bytes of memory (the "Scratchpad")
    uint8_t data[9];
    for (int i = 0; i < 9; i++) {
        data[i] = ds_bus->read();
    }

    // Run a Cyclic Redundancy Check (CRC) to guarantee the wire didn't glitch
    if (OneWire::crc8(data, 8) != data[8]) {
        return internalData; // Corrupted data packet
    }

    // Byte 0 is the Least Significant Byte (LSB)
    // Byte 1 is the Most Significant Byte (MSB)
    // Combine them into a 16-bit signed integer
    int16_t raw_temp = (data[1] << 8) | data[0];

    // The DS18B20 returns temperature in exactly 1/16ths of a degree Celsius
    internalData.temperature_c = (float)raw_temp / 16.0;
    internalData.is_valid = true;

    return internalData;
}