/*
  kcomp_BMI160.cpp - Implementation
*/

#include "kcomp_GYBMI160.h"
#include <Wire.h>
#include <math.h>

// --- Internal Variables for Z-Integration ---
static float _globalAngleZ = 0.0;        // Accumulator for Yaw
static unsigned long _lastTimeMicros = 0; // Timer for integration
static uint8_t _i2c_address = BMI160_ADDR_DEFAULT;

// --- Constants ---
// Sensitivity for ±2g (16384 LSB/g) and ±2000dps (16.4 LSB/dps)
static const float acc_scale = 16384.0; 
static const float gyr_scale = 16.4;    

// --- Registers ---
#define BMI160_REG_CHIP_ID      0x00
#define BMI160_REG_GYRO_DATA    0x0C
#define BMI160_REG_TEMP         0x20
#define BMI160_REG_CMD          0x7E

// --- Commands ---
#define CMD_ACC_NORMAL  0x11  // Power up Accel
#define CMD_GYR_NORMAL  0x15  // Power up Gyro
#define CMD_SOFT_RESET  0xB6

// --- Private Helpers ---
static void writeReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(_i2c_address);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

static uint8_t readReg(uint8_t reg) {
    Wire.beginTransmission(_i2c_address);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(_i2c_address, (uint8_t)1);
    if (Wire.available()) return Wire.read();
    return 0;
}

static void readBytes(uint8_t reg, uint8_t *buffer, uint8_t len) {
    Wire.beginTransmission(_i2c_address);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(_i2c_address, len);
    for (int i = 0; i < len; i++) {
        if (Wire.available()) buffer[i] = Wire.read();
    }
}

// --- Public Functions ---

bool initBMI160(uint8_t address) {
    _i2c_address = address;
    Wire.begin();
    delay(10);

    // 1. Check ID (Expect 0xD1)
    if (readReg(BMI160_REG_CHIP_ID) != 0xD1) {
        // Try soft reset if ID is wrong
        writeReg(BMI160_REG_CMD, CMD_SOFT_RESET);
        delay(100);
        if (readReg(BMI160_REG_CHIP_ID) != 0xD1) return false;
    }

    // 2. Wake up Sensors (Default is Suspend Mode)
    writeReg(BMI160_REG_CMD, CMD_ACC_NORMAL);
    delay(50); 
    writeReg(BMI160_REG_CMD, CMD_GYR_NORMAL);
    delay(100); 

    // 3. Reset Integration Variables
    _globalAngleZ = 0.0;
    _lastTimeMicros = micros();

    return true;
}

void resetGyroZ(void) {
    _globalAngleZ = 0.0;
}

BMI160Data readBMI160(void) {
    BMI160Data d = {0};
    uint8_t buffer[12];
    
    // 1. Calculate Delta Time (in seconds)
    unsigned long now = micros();
    float dt = (now - _lastTimeMicros) / 1000000.0;
    _lastTimeMicros = now;

    // 2. Burst Read 12 Bytes (Gyro X,Y,Z then Accel X,Y,Z)
    readBytes(BMI160_REG_GYRO_DATA, buffer, 12);

    // Parse Gyro
    int16_t gx = (buffer[1] << 8) | buffer[0];
    int16_t gy = (buffer[3] << 8) | buffer[2];
    int16_t gz = (buffer[5] << 8) | buffer[4];
    d.gyroX = gx / gyr_scale;
    d.gyroY = gy / gyr_scale;
    d.gyroZ = gz / gyr_scale;

    // Parse Accel
    int16_t ax = (buffer[7] << 8) | buffer[6];
    int16_t ay = (buffer[9] << 8) | buffer[8];
    int16_t az = (buffer[11] << 8) | buffer[10];
    d.accelX = (ax / acc_scale) * 9.81;
    d.accelY = (ay / acc_scale) * 9.81;
    d.accelZ = (az / acc_scale) * 9.81;

    // 3. Calculate Stable Angles (Pitch & Roll) using Gravity
    d.angleX = atan2(d.accelY, d.accelZ) * 180.0 / PI;
    d.angleY = atan2(-d.accelX, sqrt(d.accelY * d.accelY + d.accelZ * d.accelZ)) * 180.0 / PI;

    // 4. Calculate Z-Angle (Yaw) using Integration
    // We only add to the angle if rotation is fast enough (Deadzone > 1.0 deg/s)
    // This helps reduce drift when the sensor is sitting still.
    if (abs(d.gyroZ) > 1.0) { 
        _globalAngleZ += d.gyroZ * dt;
    }
    d.angleZ = _globalAngleZ;

    // 5. Read Temperature
    uint8_t tempBuf[2];
    readBytes(BMI160_REG_TEMP, tempBuf, 2);
    int16_t rawTemp = (tempBuf[1] << 8) | tempBuf[0];
    d.temperature = 23.0 + (rawTemp / 512.0);

    return d;
}