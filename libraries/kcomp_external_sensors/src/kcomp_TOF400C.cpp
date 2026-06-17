#include "kcomp_TOF400C.h"
#include <Wire.h>

#define ADDRESS_DEFAULT 0x29

// --- Pololu VL53L1X Register Map Defines ---
#define SOFT_RESET                                      0x0000
#define OSC_MEASURED__FAST_OSC__FREQUENCY               0x0006
#define VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND           0x0008
#define VHV_CONFIG__COUNT_THRESH                        0x000B
#define ALGO__PART_TO_PART_RANGE_OFFSET_MM              0x001E
#define MM_CONFIG__OUTER_OFFSET_MM                      0x0022
#define DSS_CONFIG__TARGET_TOTAL_RATE_MCPS              0x0024
#define GPIO__TIO_HV_STATUS                             0x0031
#define SIGMA_ESTIMATOR__EFFECTIVE_PULSE_WIDTH_NS       0x0036
#define SIGMA_ESTIMATOR__EFFECTIVE_AMBIENT_WIDTH_NS     0x0037
#define ALGO__CROSSTALK_COMPENSATION_VALID_HEIGHT_MM    0x0039
#define ALGO__RANGE_IGNORE_VALID_HEIGHT_MM              0x003E
#define ALGO__RANGE_MIN_CLIP                            0x003F
#define ALGO__CONSISTENCY_CHECK__TOLERANCE              0x0040
#define PHASECAL_CONFIG__TIMEOUT_MACROP                 0x004B
#define DSS_CONFIG__ROI_MODE_CONTROL                    0x004F
#define SYSTEM__THRESH_RATE_HIGH                        0x0050
#define SYSTEM__THRESH_RATE_LOW                         0x0052
#define DSS_CONFIG__MANUAL_EFFECTIVE_SPADS_SELECT       0x0054
#define DSS_CONFIG__APERTURE_ATTENUATION                0x0057
#define RANGE_CONFIG__TIMEOUT_MACROP_A                  0x005E
#define RANGE_CONFIG__VCSEL_PERIOD_A                    0x0060
#define RANGE_CONFIG__TIMEOUT_MACROP_B                  0x0061
#define RANGE_CONFIG__VCSEL_PERIOD_B                    0x0063
#define RANGE_CONFIG__SIGMA_THRESH                      0x0064
#define RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS     0x0066
#define RANGE_CONFIG__VALID_PHASE_HIGH                  0x0069
#define SYSTEM__INTERMEASUREMENT_PERIOD                 0x006C
#define SYSTEM__GROUPED_PARAMETER_HOLD_0                0x0071
#define SYSTEM__SEED_CONFIG                             0x0077
#define SD_CONFIG__WOI_SD0                              0x0078
#define SD_CONFIG__INITIAL_PHASE_SD0                    0x007A
#define SYSTEM__GROUPED_PARAMETER_HOLD_1                0x007C
#define SD_CONFIG__QUANTIFIER                           0x007E
#define SYSTEM__SEQUENCE_CONFIG                         0x0081
#define SYSTEM__GROUPED_PARAMETER_HOLD                  0x0082
#define SYSTEM__INTERRUPT_CLEAR                         0x0086
#define SYSTEM__MODE_START                              0x0087
#define RESULT__OSC_CALIBRATE_VAL                       0x00DE
#define FIRMWARE__SYSTEM_STATUS                         0x00E5
#define IDENTIFICATION__MODEL_ID                        0x010F

static tof400c_data_t internalData;

// --- Private I2C Helpers ---
static void writeReg(uint16_t reg, uint8_t value) {
    Wire.beginTransmission(ADDRESS_DEFAULT);
    Wire.write(reg >> 8);
    Wire.write(reg & 0xFF);
    Wire.write(value);
    Wire.endTransmission();
}

static void writeReg16Bit(uint16_t reg, uint16_t value) {
    Wire.beginTransmission(ADDRESS_DEFAULT);
    Wire.write(reg >> 8);
    Wire.write(reg & 0xFF);
    Wire.write(value >> 8);
    Wire.write(value & 0xFF);
    Wire.endTransmission();
}

static void writeReg32Bit(uint16_t reg, uint32_t value) {
    Wire.beginTransmission(ADDRESS_DEFAULT);
    Wire.write(reg >> 8);
    Wire.write(reg & 0xFF);
    Wire.write((value >> 24) & 0xFF);
    Wire.write((value >> 16) & 0xFF);
    Wire.write((value >> 8) & 0xFF);
    Wire.write(value & 0xFF);
    Wire.endTransmission();
}

static uint8_t readReg(uint16_t reg) {
    Wire.beginTransmission(ADDRESS_DEFAULT);
    Wire.write(reg >> 8);
    Wire.write(reg & 0xFF);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)ADDRESS_DEFAULT, (uint8_t)1);
    return Wire.read();
}

static uint16_t readReg16Bit(uint16_t reg) {
    Wire.beginTransmission(ADDRESS_DEFAULT);
    Wire.write(reg >> 8);
    Wire.write(reg & 0xFF);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)ADDRESS_DEFAULT, (uint8_t)2);
    uint16_t val = (uint16_t)Wire.read() << 8;
    val |= Wire.read();
    return val;
}

// --- Public Functions ---

bool initTOF400C() {
    Wire.begin();

    if (readReg16Bit(IDENTIFICATION__MODEL_ID) != 0xEACC) return false;

    writeReg(SOFT_RESET, 0x00);
    delay(100);
    writeReg(SOFT_RESET, 0x01);
    delay(1);

    uint16_t timeout = 0;
    while ((readReg(FIRMWARE__SYSTEM_STATUS) & 0x01) == 0) {
        delay(1);
        if (++timeout > 100) return false;
    }

    uint16_t fast_osc_frequency = readReg16Bit(OSC_MEASURED__FAST_OSC__FREQUENCY);

    writeReg16Bit(VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND, fast_osc_frequency * 2000);
    writeReg(VHV_CONFIG__COUNT_THRESH, 43);
    writeReg16Bit(DSS_CONFIG__TARGET_TOTAL_RATE_MCPS, 0x0A00);
    writeReg(GPIO__TIO_HV_STATUS, 0x02);
    writeReg(SIGMA_ESTIMATOR__EFFECTIVE_PULSE_WIDTH_NS, 8);
    writeReg(SIGMA_ESTIMATOR__EFFECTIVE_AMBIENT_WIDTH_NS, 16);
    writeReg(ALGO__CROSSTALK_COMPENSATION_VALID_HEIGHT_MM, 0x01);
    writeReg(ALGO__RANGE_IGNORE_VALID_HEIGHT_MM, 0xFF);
    writeReg(ALGO__RANGE_MIN_CLIP, 0);
    writeReg(ALGO__CONSISTENCY_CHECK__TOLERANCE, 2);
    writeReg16Bit(SYSTEM__THRESH_RATE_HIGH, 0x0000);
    writeReg16Bit(SYSTEM__THRESH_RATE_LOW, 0x0000);
    writeReg(DSS_CONFIG__APERTURE_ATTENUATION, 0x38);
    writeReg16Bit(RANGE_CONFIG__SIGMA_THRESH, 360);
    writeReg16Bit(RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS, 192);
    writeReg(SYSTEM__GROUPED_PARAMETER_HOLD_0, 0x01);
    writeReg(SYSTEM__GROUPED_PARAMETER_HOLD_1, 0x01);
    writeReg(SD_CONFIG__QUANTIFIER, 2);
    writeReg(SYSTEM__GROUPED_PARAMETER_HOLD, 0x00);
    writeReg(SYSTEM__SEED_CONFIG, 1);
    writeReg(SYSTEM__SEQUENCE_CONFIG, 0x8B);
    writeReg16Bit(DSS_CONFIG__MANUAL_EFFECTIVE_SPADS_SELECT, 200 << 8);
    writeReg(DSS_CONFIG__ROI_MODE_CONTROL, 2);

    // FIX: Apply Pololu setupManualCalibration() -> Part-to-part hardware offset
    writeReg16Bit(ALGO__PART_TO_PART_RANGE_OFFSET_MM, readReg16Bit(MM_CONFIG__OUTER_OFFSET_MM) * 4);

    setTOF400CMode(TOF400C_MODE_LONG);

    return true;
}

void setTOF400CMode(TOF400C_Mode_t mode) {
    writeReg(SYSTEM__MODE_START, 0x00); // Stop ranging before config
    
    switch(mode) {
        case TOF400C_MODE_SHORT:
            writeReg(PHASECAL_CONFIG__TIMEOUT_MACROP, 0x14); 
            writeReg(RANGE_CONFIG__VCSEL_PERIOD_A, 0x07); 
            writeReg(RANGE_CONFIG__VCSEL_PERIOD_B, 0x05); 
            writeReg(RANGE_CONFIG__VALID_PHASE_HIGH, 0x38); 
            writeReg16Bit(SD_CONFIG__WOI_SD0, 0x0705); 
            writeReg16Bit(SD_CONFIG__INITIAL_PHASE_SD0, 0x0606); 
            writeReg16Bit(RANGE_CONFIG__TIMEOUT_MACROP_A, 0x014D);
            writeReg16Bit(RANGE_CONFIG__TIMEOUT_MACROP_B, 0x014D);
            break;
            
        case TOF400C_MODE_MEDIUM:
            writeReg(PHASECAL_CONFIG__TIMEOUT_MACROP, 0x0A); 
            writeReg(RANGE_CONFIG__VCSEL_PERIOD_A, 0x0B); 
            writeReg(RANGE_CONFIG__VCSEL_PERIOD_B, 0x09); 
            writeReg(RANGE_CONFIG__VALID_PHASE_HIGH, 0x78); 
            writeReg16Bit(SD_CONFIG__WOI_SD0, 0x0B09); 
            writeReg16Bit(SD_CONFIG__INITIAL_PHASE_SD0, 0x0A0A); 
            writeReg16Bit(RANGE_CONFIG__TIMEOUT_MACROP_A, 0x01CC);
            writeReg16Bit(RANGE_CONFIG__TIMEOUT_MACROP_B, 0x01CC);
            break;
            
        case TOF400C_MODE_LONG:
            writeReg(PHASECAL_CONFIG__TIMEOUT_MACROP, 0x0A); 
            writeReg(RANGE_CONFIG__VCSEL_PERIOD_A, 0x0F); 
            writeReg(RANGE_CONFIG__VCSEL_PERIOD_B, 0x0D); 
            writeReg(RANGE_CONFIG__VALID_PHASE_HIGH, 0xB8); 
            writeReg16Bit(SD_CONFIG__WOI_SD0, 0x0F0D); 
            writeReg16Bit(SD_CONFIG__INITIAL_PHASE_SD0, 0x0E0E); 
            writeReg16Bit(RANGE_CONFIG__TIMEOUT_MACROP_A, 0x01CC);
            writeReg16Bit(RANGE_CONFIG__TIMEOUT_MACROP_B, 0x01CC);
            break;
    }

    // FIX: Set Intermeasurement Period using the correct Clock register (0x00DE)
    uint16_t osc_calibrate = readReg16Bit(RESULT__OSC_CALIBRATE_VAL);
    writeReg32Bit(SYSTEM__INTERMEASUREMENT_PERIOD, 50 * (uint32_t)osc_calibrate);

    writeReg(SYSTEM__INTERRUPT_CLEAR, 0x01); 
    writeReg(SYSTEM__MODE_START, 0x40); 
}

tof400c_data_t readTOF400C() {
    internalData.is_valid = false;

    // Check Data Ready Status (Active LOW: 0 = Ready)
    if ((readReg(GPIO__TIO_HV_STATUS) & 0x01) != 0) {
        return internalData; 
    }

    // --------------------------------------------------------------------------------
    // FIX 1: Pololu readResults() - 17-Byte Atomic Burst Read
    // Must be read sequentially to ensure the I2C registers don't shift mid-read.
    // --------------------------------------------------------------------------------
    Wire.beginTransmission(ADDRESS_DEFAULT);
    Wire.write(0x00); // Address High
    Wire.write(0x89); // Address Low (RESULT__RANGE_STATUS)
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)ADDRESS_DEFAULT, (uint8_t)17);

    uint8_t status = Wire.read() & 0x1F;
    Wire.read(); // report_status (discarded)
    Wire.read(); // stream_count (discarded)
    uint16_t spads = (uint16_t)Wire.read() << 8 | Wire.read();
    uint16_t peak_signal = (uint16_t)Wire.read() << 8 | Wire.read();
    Wire.read(); Wire.read(); // ambient_count (discarded)
    Wire.read(); Wire.read(); // sigma (discarded)
    Wire.read(); Wire.read(); // phase (discarded)
    uint16_t range = (uint16_t)Wire.read() << 8 | Wire.read(); // Final Range SD0

    // Clear hardware interrupt so the background timer starts the next measurement
    writeReg(SYSTEM__INTERRUPT_CLEAR, 0x01); 

    // --------------------------------------------------------------------------------
    // FIX 2: Pololu updateDSS() - Dynamic SPAD Selection
    // Dynamically adjust sensitivity to prevent saturation/inaccuracy on the NEXT loop
    // --------------------------------------------------------------------------------
    if (spads != 0) {
        uint32_t totalRate = ((uint32_t)peak_signal * 256) / spads;
        if (totalRate > 0) {
            uint32_t requiredSpads = ((uint32_t)0x0A00 * 256) / totalRate; // Target = 0x0A00
            if (requiredSpads > 0xFFFF) requiredSpads = 0xFFFF;
            writeReg16Bit(DSS_CONFIG__MANUAL_EFFECTIVE_SPADS_SELECT, (uint16_t)requiredSpads);
        }
    }

    // Pololu accepts 0, 9, 4, and 7 as valid measurement states
    if (range > 0 && (status == 0 || status == 9 || status == 4 || status == 7)) {
        internalData.distance_mm = range;
        internalData.is_valid = true;
    }

    return internalData;
}