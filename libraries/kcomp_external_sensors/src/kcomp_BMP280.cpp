#include "kcomp_BMP280.h"
#include <Wire.h>

// --- Calibration Data (Compensation Parameters) ---
// These are read from the sensor at startup and used for math.
static uint16_t dig_T1;
static int16_t  dig_T2, dig_T3;
static uint16_t dig_P1;
static int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;

// "Fine" temperature variable for pressure compensation
static int32_t t_fine; 

// --- Private Helper Functions ---

static void writeRegister8(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(BMP280_I2CADDR_DEFAULT);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

static uint8_t readRegister8(uint8_t reg) {
    Wire.beginTransmission(BMP280_I2CADDR_DEFAULT);
    Wire.write(reg);
    Wire.endTransmission();
    
    Wire.requestFrom(BMP280_I2CADDR_DEFAULT, 1);
    return Wire.read();
}

static uint16_t readRegister16LE(uint8_t reg) {
    Wire.beginTransmission(BMP280_I2CADDR_DEFAULT);
    Wire.write(reg);
    Wire.endTransmission();
    
    Wire.requestFrom(BMP280_I2CADDR_DEFAULT, 2);
    return (Wire.read() | (Wire.read() << 8));
}

static int16_t readRegisterS16LE(uint8_t reg) {
    return (int16_t)readRegister16LE(reg);
}

static void readCalibrationData() {
    dig_T1 = readRegister16LE(0x88);
    dig_T2 = readRegisterS16LE(0x8A);
    dig_T3 = readRegisterS16LE(0x8C);

    dig_P1 = readRegister16LE(0x8E);
    dig_P2 = readRegisterS16LE(0x90);
    dig_P3 = readRegisterS16LE(0x92);
    dig_P4 = readRegisterS16LE(0x94);
    dig_P5 = readRegisterS16LE(0x96);
    dig_P6 = readRegisterS16LE(0x98);
    dig_P7 = readRegisterS16LE(0x9A);
    dig_P8 = readRegisterS16LE(0x9C);
    dig_P9 = readRegisterS16LE(0x9E);
}

// --- Public Functions ---

bool initBMP280() {
    Wire.begin();

    // Check Chip ID to ensure connection
    if (readRegister8(BMP280_REG_ID) != BMP280_CHIP_ID) {
        return false; 
    }

    readCalibrationData();

    // Configure sensor:
    // osrs_t (temp oversampling) x1 = 001
    // osrs_p (press oversampling) x1 = 001
    // mode Normal = 11
    // Register 0xF4 = (001 << 5) | (001 << 2) | 11 = 0x27
    writeRegister8(BMP280_REG_CTRL_MEAS, 0x27);
    
    // Config: 
    // filter = off (000), t_sb = 0.5ms (000)
    // Register 0xF5 = 0x00
    writeRegister8(BMP280_REG_CONFIG, 0x00);

    return true;
}

float getBMP280Temperature() {
    // Read raw ADC values (20-bit)
    // We read 3 registers starting at 0xFA
    Wire.beginTransmission(BMP280_I2CADDR_DEFAULT);
    Wire.write(BMP280_REG_TEMP_MSB);
    Wire.endTransmission();
    
    Wire.requestFrom(BMP280_I2CADDR_DEFAULT, 3);
    int32_t msb = Wire.read();
    int32_t lsb = Wire.read();
    int32_t xlsb = Wire.read();
    
    int32_t adc_T = (msb << 12) | (lsb << 4) | (xlsb >> 4);

    // Compensation Formula (from datasheet)
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;

    float T = (t_fine * 5 + 128) >> 8;
    return T / 100.0;
}

float getBMP280Pressure() {
    // Ensure t_fine is updated (must read temp first)
    getBMP280Temperature(); 

    // Read raw ADC values (20-bit)
    Wire.beginTransmission(BMP280_I2CADDR_DEFAULT);
    Wire.write(BMP280_REG_PRESS_MSB);
    Wire.endTransmission();
    
    Wire.requestFrom(BMP280_I2CADDR_DEFAULT, 3);
    int32_t msb = Wire.read();
    int32_t lsb = Wire.read();
    int32_t xlsb = Wire.read();
    
    int32_t adc_P = (msb << 12) | (lsb << 4) | (xlsb >> 4);

    // Compensation Formula (from datasheet)
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0) return 0; // Avoid divide by zero

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

    return (float)p / 256.0;
}

float getBMP280Altitude(float seaLevelhPa) {
    float pressure = getBMP280Pressure(); // Returns Pa
    pressure /= 100.0; // Convert to hPa
    
    // Hypsometric Formula
    return 44330.0 * (1.0 - pow(pressure / seaLevelhPa, 0.1903));
}