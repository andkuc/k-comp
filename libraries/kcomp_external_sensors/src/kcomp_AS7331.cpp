#include "kcomp_AS7331.h"
#include <Wire.h>

// --- AS7331 Baseline Constants ---
static const float FSR_UVA_BASE = 340.0f; 
static const float FSR_UVB_BASE = 378.0f; 
static const float FSR_UVC_BASE = 166.0f; 

// Current configuration states
static uint8_t currentGainReg = AS7331_GAIN_64X; 
static uint8_t currentTimeReg = 6;               

// --- Private Helper Functions ---

static void writeRegister8(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(AS7331_I2CADDR_DEFAULT);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

static uint16_t readRegister16LE(uint8_t reg) {
    Wire.beginTransmission(AS7331_I2CADDR_DEFAULT);
    Wire.write(reg);
    Wire.endTransmission(false); // Repeated start for stable reads
    
    // Safely request 2 bytes
    if (Wire.requestFrom((uint8_t)AS7331_I2CADDR_DEFAULT, (uint8_t)2) != 2) {
        return 0; 
    }
    
    uint16_t val = Wire.read();
    val |= (Wire.read() << 8);
    return val;
}

static float calculateConversionFactor() {
    float gainFactor = (float)(1 << (11 - currentGainReg)); 
    float timeFactor = (float)(1 << currentTimeReg);
    return 1.0f / (gainFactor * timeFactor);
}

// --- Public Functions ---

bool initAS7331() {
    Wire.begin();

    // Verify I2C connection
    Wire.beginTransmission(AS7331_I2CADDR_DEFAULT);
    if (Wire.endTransmission() != 0) {
        return false; 
    }

    // 1. Software Reset to clear stuck states
    writeRegister8(AS7331_REG_OSR, 0x0A);
    delay(10);

    // 2. Set to Configuration Mode (OSR = 0x02)
    writeRegister8(AS7331_REG_OSR, 0x02);
    delay(10);
    
    // 3. Combine Gain (Upper 4 bits) and Time (Lower 4 bits) into CREG1
    currentGainReg = AS7331_GAIN_64X;
    currentTimeReg = 0x06; 
    uint8_t creg1_val = (currentGainReg << 4) | (currentTimeReg & 0x0F);
    writeRegister8(AS7331_REG_CREG1, creg1_val); 
    
    // 4. THE FIX: Set Continuous Mode and Default Clock (CREG3 = 0x00)
    writeRegister8(AS7331_REG_CREG3, 0x00);

    // 5. Switch to Measurement Mode (Step 1)
    writeRegister8(AS7331_REG_OSR, 0x03); 
    delay(10);                            
    // Trigger Continuous Start (Step 2)
    writeRegister8(AS7331_REG_OSR, 0x83); 
    delay(10);

    return true;
}

void setAS7331Gain(uint8_t gainReg) {
    if (gainReg > 11) gainReg = 11; 
    currentGainReg = gainReg;

    writeRegister8(AS7331_REG_OSR, 0x02); // Switch to config mode
    delay(10); 

    // Update CREG1 with new Gain and existing Time
    uint8_t creg1_val = (currentGainReg << 4) | (currentTimeReg & 0x0F);
    writeRegister8(AS7331_REG_CREG1, creg1_val);

    // Return to Measurement Mode & Trigger Start
    writeRegister8(AS7331_REG_OSR, 0x03); 
    delay(10);
    writeRegister8(AS7331_REG_OSR, 0x83); 
    delay(10);
}

// --- Calculated Data ---
float getAS7331_UVA() { return (float)getRawUVA() * FSR_UVA_BASE * calculateConversionFactor(); }
float getAS7331_UVB() { return (float)getRawUVB() * FSR_UVB_BASE * calculateConversionFactor(); }
float getAS7331_UVC() { return (float)getRawUVC() * FSR_UVC_BASE * calculateConversionFactor(); }

// --- Raw Data ---
uint16_t getRawUVA() { return readRegister16LE(AS7331_REG_UVA); }
uint16_t getRawUVB() { return readRegister16LE(AS7331_REG_UVB); }
uint16_t getRawUVC() { return readRegister16LE(AS7331_REG_UVC); }