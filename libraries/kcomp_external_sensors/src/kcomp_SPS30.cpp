#include "kcomp_SPS30.h"
#include <Wire.h>

// The single, globally accessible (within this file) data instance
static sps30_data_t internalData;

// --- Private Helper Functions ---
static uint8_t calcCrc(uint8_t data[2]) {
    uint8_t crc = 0xFF;
    for(int i = 0; i < 2; i++) {
        crc ^= data[i];
        for(uint8_t bit = 8; bit > 0; --bit) {
            if(crc & 0x80) crc = (crc << 1) ^ 0x31;
            else crc = (crc << 1);
        }
    }
    return crc;
}

static void sendCmd(uint16_t cmd) {
    Wire.beginTransmission(SPS30_I2CADDR_DEFAULT);
    Wire.write(cmd >> 8);
    Wire.write(cmd & 0xFF);
    Wire.endTransmission();
}

static void sendCmdArg(uint16_t cmd, uint16_t arg) {
    Wire.beginTransmission(SPS30_I2CADDR_DEFAULT);
    Wire.write(cmd >> 8);
    Wire.write(cmd & 0xFF);
    Wire.write(arg >> 8);
    Wire.write(arg & 0xFF);
    
    uint8_t data[2] = {(uint8_t)(arg >> 8), (uint8_t)(arg & 0xFF)};
    Wire.write(calcCrc(data));
    Wire.endTransmission();
}

// --- Core Functions ---

bool initSPS30() {
    Wire.begin();
    Wire.beginTransmission(SPS30_I2CADDR_DEFAULT);
    if (Wire.endTransmission() != 0) return false; 

    // Start Measurement in INTEGER Mode
    sendCmdArg(0x0010, 0x0500);
    delay(50); 
    
    return true;
}

sps30_data_t readSPS30() {
    // Default to false at the start of every read attempt
    internalData.is_valid = false;

    sendCmd(0x0202);
    Wire.requestFrom((uint8_t)SPS30_I2CADDR_DEFAULT, (uint8_t)3);
    if (Wire.available() < 3) return internalData;
    
    Wire.read(); // MSB
    uint8_t lsb = Wire.read(); // LSB
    Wire.read(); // CRC 
    
    // If LSB is 0, no new data is ready yet
    if (lsb == 0x00) return internalData; 
    
    sendCmd(0x0300);
    Wire.requestFrom((uint8_t)SPS30_I2CADDR_DEFAULT, (uint8_t)30);
    if (Wire.available() < 30) return internalData; 
    
    uint16_t raw_vals[10];
    for(int i = 0; i < 10; i++) {
        uint8_t msb = Wire.read();
        uint8_t lsb = Wire.read();
        Wire.read(); // Discard CRC
        raw_vals[i] = (msb << 8) | lsb;
    }
    
    // Populate the global struct
    internalData.pm1_0  = raw_vals[0];
    internalData.pm2_5  = raw_vals[1];
    internalData.pm4_0  = raw_vals[2];
    internalData.pm10_0 = raw_vals[3];
    internalData.nc0_5  = raw_vals[4];
    internalData.nc1_0  = raw_vals[5];
    internalData.nc2_5  = raw_vals[6];
    internalData.nc4_0  = raw_vals[7];
    internalData.nc10_0 = raw_vals[8];
    internalData.typical_particle_size = (float)raw_vals[9] / 1000.0f;
    
    // Mark as valid and return it!
    internalData.is_valid = true;
    return internalData;
}

// --- Smart Analysis Functions ---

SPS30_AQI_t getSPS30_AirQualityLevel() {
    if (internalData.pm2_5 <= 12)  return SPS30_AQI_GOOD;
    if (internalData.pm2_5 <= 35)  return SPS30_AQI_MODERATE;
    if (internalData.pm2_5 <= 55)  return SPS30_AQI_UNHEALTHY_SENSITIVE;
    if (internalData.pm2_5 <= 150) return SPS30_AQI_UNHEALTHY;
    if (internalData.pm2_5 <= 250) return SPS30_AQI_VERY_UNHEALTHY;
    return SPS30_AQI_HAZARDOUS;
}

SPS30_Source_t getSPS30_PollutionSource() {
    if (internalData.pm2_5 < 12) return SPS30_SOURCE_CLEAN;
    if (internalData.pm10_0 == 0) return SPS30_SOURCE_CLEAN;

    float ratio = (float)internalData.pm1_0 / (float)internalData.pm10_0;

    if (ratio >= 0.70) return SPS30_SOURCE_COMBUSTION;
    if (ratio <= 0.40) return SPS30_SOURCE_DUST;

    return SPS30_SOURCE_MIXED;
}

uint8_t getSPS30_AirQualityScore() {
    int score = 100 - (internalData.pm2_5 * 2);
    if (score < 0) return 0;
    if (score > 100) return 100;
    return (uint8_t)score;
}