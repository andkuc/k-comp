#include "kcomp_AHT10.h"

#include <Wire.h>

// --- Internal Helper Function (Private to this file) ---
static unsigned long readAHT10Raw(bool wantHumidity) {
    // 1. Send Trigger Measurement Command
    Wire.beginTransmission(AHT10_I2CADDR_DEFAULT);
    Wire.write(AHT10_CMD_TRIGGER);
    Wire.write(0x33);
    Wire.write(0x00);
    
    Wire.endTransmission();

    // 2. Wait for measurement (datasheet requires >75ms)
    delay(100);

    // 3. Request 6 bytes of data
    Wire.requestFrom(AHT10_I2CADDR_DEFAULT, 6);
    
    if (Wire.available() != 6) {
        return 0; // Return 0 on error
    }

    uint8_t data[6];
    for(int i = 0; i < 6; i++) {
        data[i] = Wire.read();
    }

    // 4. Parse 20-bit data
    unsigned long rawValue = 0;
    if (wantHumidity) {
        // Humidity: Bytes 1, 2, and top 4 bits of 3
        rawValue = ((unsigned long)data[1] << 12) | ((unsigned long)data[2] << 4) | (data[3] >> 4);
    } else {
        // Temperature: Lower 4 bits of byte 3, byte 4, and byte 5
        rawValue = ((unsigned long)(data[3] & 0x0F) << 16) | ((unsigned long)data[4] << 8) | data[5];
    }
    return rawValue;
}

// --- Public Functions ---

void initAHT10() {
    Wire.begin();
    
    // Send Calibration Command
    Wire.beginTransmission(AHT10_I2CADDR_DEFAULT);
    Wire.write(AHT10_CMD_CALIBRATE);
    Wire.write(0x08);
    Wire.write(0x00);
    Wire.endTransmission();
    
    delay(500); // Allow time to calibrate
}

void resetAHT10() {
    Wire.beginTransmission(AHT10_I2CADDR_DEFAULT);
    Wire.write(AHT10_CMD_SOFTRESET);
    Wire.endTransmission();
    delay(20);
}

float getAHT10Temperature() {
    unsigned long raw = readAHT10Raw(false);
    // Formula: ((Value * 200) / 2^20) - 50
    return ((float)raw * 200.0 / 1048576.0) - 50.0;
}

float getAHT10Humidity() {
    unsigned long raw = readAHT10Raw(true);
    // Formula: (Value * 100) / 2^20
    return (float)raw * 100.0 / 1048576.0;
}