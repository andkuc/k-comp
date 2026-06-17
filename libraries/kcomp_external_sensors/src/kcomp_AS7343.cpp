#include "kcomp_AS7343.h"
#include <Wire.h>

#define AS7343_I2C_ADDR (uint8_t)0x39

// --- AS7343 Registers ---
#define REG_ENABLE    0x80
#define REG_ATIME     0x81
#define REG_STATUS2   0x90  
#define REG_DATA_START 0x95 
#define REG_CFG0      0xBF  
#define REG_CFG1      0xC6  
#define REG_LED       0xCD  
#define REG_ASTEP_L   0xD4  
#define REG_ASTEP_H   0xD5  
#define REG_CFG20     0xD6  

// --- Internal State ---
static AS7343Data internalData = {0};
static bool isInitialized = false;

// --- Low-Level I2C Helpers ---

static void writeReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(AS7343_I2C_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission(); 
}

static uint8_t readReg(uint8_t reg) {
    Wire.beginTransmission(AS7343_I2C_ADDR);
    Wire.write(reg);
    // CRITICAL FIX: Standard endTransmission prevents infinite I2C lockups!
    Wire.endTransmission(); 
    Wire.requestFrom((uint8_t)AS7343_I2C_ADDR, (uint8_t)1);
    if (Wire.available()) return Wire.read();
    return 0;
}
// --- Standard Functions ---

bool initAS7343(void) {
    Wire.begin();
    
    Wire.beginTransmission(AS7343_I2C_ADDR);
    if (Wire.endTransmission() != 0) {
        isInitialized = false;
        return false;
    }

    // Explicitly set Bank 0 without risking a read-modify-write freeze
    writeReg(REG_CFG0, 0x00); 

    // Power ON (PON)
    writeReg(REG_ENABLE, 0x01);
    delay(10); 

    // Set integration time for a sensible ~50ms cycle
    writeReg(REG_ATIME, 29);
    writeReg(REG_ASTEP_L, 599 & 0xFF);
    writeReg(REG_ASTEP_H, (599 >> 8) & 0xFF);
    
    // Set Gain directly to AGAIN_32X (6)
    writeReg(REG_CFG1, 6); 

    // Configure Auto-SMUX to 18-channel mode
    writeReg(REG_CFG20, 0x60); 

    // Start Continuous Measurement (PON + SPEN)
    writeReg(REG_ENABLE, 0x03); 

    isInitialized = true;
    return true;
}

bool updateAS7343(void) {
    if (!isInitialized) return false;
    internalData.updated = false;
    
    // Poll AVALID bit (Data Valid) in STATUS2 register
    uint32_t start = millis();
    while (!(readReg(REG_STATUS2) & 0x40)) { 
        if (millis() - start > 800) { 
            return false; 
        }
        delay(5); 
    }

    uint8_t buffer[36];

    // Read First Half (18 bytes)
    Wire.beginTransmission(AS7343_I2C_ADDR);
    Wire.write(REG_DATA_START);
    Wire.endTransmission(); // CRITICAL FIX
    Wire.requestFrom((uint8_t)AS7343_I2C_ADDR, (uint8_t)18);
    for (int i = 0; i < 18; i++) {
        buffer[i] = Wire.available() ? Wire.read() : 0;
    }

    // Read Second Half (18 bytes)
    Wire.beginTransmission(AS7343_I2C_ADDR);
    Wire.write(REG_DATA_START + 18);
    Wire.endTransmission(); // CRITICAL FIX
    Wire.requestFrom((uint8_t)AS7343_I2C_ADDR, (uint8_t)18);
    for (int i = 18; i < 36; i++) {
        buffer[i] = Wire.available() ? Wire.read() : 0;
    }

    uint16_t ch[18];
    for (int i = 0; i < 18; i++) {
        ch[i] = buffer[i * 2] | (buffer[i * 2 + 1] << 8);
    }

    // Align mapping with reference driver
    internalData.fz_450nm = ch[0];
    internalData.fy_555nm = ch[1];
    internalData.fxl_600nm = ch[2];
    internalData.nir = ch[3];
    internalData.clear = (ch[4] + ch[10] + ch[16]) / 3; 
    internalData.f2_425nm = ch[6];
    internalData.f3_475nm = ch[7];
    internalData.f4_515nm = ch[8];
    internalData.f6_640nm = ch[9];
    internalData.f1_405nm = ch[12];
    internalData.f7_690nm = ch[13];
    internalData.f8_745nm = ch[14];
    internalData.f5_550nm = ch[15];
    
    internalData.valid = true;
    internalData.updated = true;    
    return true;
}

AS7343Data getAS7343Data(void) {
    return internalData;
}

void setAS7343LED(bool enable, uint8_t current_mA) {
    if (!isInitialized) return;
    
    if (current_mA < 4) current_mA = 4;
    if (current_mA > 258) current_mA = 258;
    
    uint8_t c_val = (current_mA - 4) / 2; 
    
    if (enable) {
        writeReg(REG_LED, 0x80 | c_val);
    } else {
        writeReg(REG_LED, c_val);
    }
}

void setAS7343Gain(uint8_t gain_level) {
    if (!isInitialized) return;
    if (gain_level > 12) gain_level = 12;
    uint8_t cfg1 = readReg(REG_CFG1);
    writeReg(REG_CFG1, (cfg1 & 0xE0) | gain_level); 
}
