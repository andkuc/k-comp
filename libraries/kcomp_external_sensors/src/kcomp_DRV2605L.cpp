#include "kcomp_DRV2605L.h"
#include <Wire.h>

#define DRV2605_ADDRESS   0x5A

#define REG_MODE          0x01
#define REG_LIBRARY       0x03
#define REG_WAVESEQ1      0x04
#define REG_WAVESEQ2      0x05
#define REG_GO            0x0C
#define REG_AUDIOCTRL     0x11
#define REG_CONTROL1      0x1B
#define REG_CONTROL3      0x1D

// Internal I2C Helpers
static void writeRegister8(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(DRV2605_ADDRESS);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

static uint8_t readRegister8(uint8_t reg) {
    Wire.beginTransmission(DRV2605_ADDRESS);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)DRV2605_ADDRESS, (uint8_t)1);
    return Wire.read();
}

bool initDRV2605L() {
    Wire.begin();
    
    Wire.beginTransmission(DRV2605_ADDRESS);
    if (Wire.endTransmission() != 0) return false; 

    // Take out of standby mode and set to internal trigger (0x00)
    writeRegister8(REG_MODE, 0x00); 
    
    // Select Library 1 (Optimized for standard ERM motors)
    writeRegister8(REG_LIBRARY, 1); 

    return true;
}

void playHapticEffect(uint8_t effectID) {
    writeRegister8(REG_WAVESEQ1, effectID);
    writeRegister8(REG_WAVESEQ2, 0); 
    writeRegister8(REG_GO, 1);
}

void stopHaptic() {
    writeRegister8(REG_GO, 0);
}

void setHapticSequenceSlot(uint8_t slotNumber, uint8_t effectID) {
    if (slotNumber > 7) return; 
    writeRegister8(REG_WAVESEQ1 + slotNumber, effectID);
}

void playHapticSequence() {
    writeRegister8(REG_GO, 1);
}

// --- NEW: AUDIO FEATURE ---

void enableAudioMode() {
    // 1. Set AC Coupling (Bit 5 = 1 in Control 1)
    // This allows raw audio signals (which alternate positive/negative) to be read safely.
    uint8_t ctrl1 = readRegister8(REG_CONTROL1);
    writeRegister8(REG_CONTROL1, ctrl1 | 0x20);

    // 2. Set Analog Input Mode (Bit 0 = 0 in Control 3)
    uint8_t ctrl3 = readRegister8(REG_CONTROL3);
    writeRegister8(REG_CONTROL3, ctrl3 & 0xFE);

    // 3. Switch device Mode to Audio-To-Vibe (0x04)
    writeRegister8(REG_MODE, 0x04);
}

void enableInternalROMMode() {
    // Switch device Mode back to Internal Trigger (0x00)
    writeRegister8(REG_MODE, 0x00);
}

#define REG_RTP_INPUT 0x02 // The Real-Time Playback register

void enableRealTimeMode() {
    // Switch device Mode to Real-Time Playback (0x05)
    writeRegister8(REG_MODE, 0x05);
}

void setRealTimeIntensity(uint8_t intensity) {
    // Write the raw power value (0-255) directly to the RTP register
    writeRegister8(REG_RTP_INPUT, intensity);
}