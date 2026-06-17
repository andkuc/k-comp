#ifndef _KCOMP_AHT10_H
#define _KCOMP_AHT10_H

#include "Arduino.h"

// --- Definitions ---
#define AHT10_I2CADDR_DEFAULT   0x38   ///< AHT10 default i2c address
#define AHT10_CMD_CALIBRATE     0xE1   ///< Calibration command
#define AHT10_CMD_TRIGGER       0xAC   ///< Trigger reading command
#define AHT10_CMD_SOFTRESET     0xBA   ///< Soft reset command
#define AHT10_STATUS_BUSY       0x80   ///< Status bit for busy
#define AHT10_STATUS_CALIBRATED 0x08   ///< Status bit for calibrated

// --- Function Prototypes ---

// Initialize the sensor (starts Wire and sends calibration)
void initAHT10();

// Resets the sensor
void resetAHT10();

// Read Temperature in Celsius
float getAHT10Temperature();

// Read Humidity in %RH
float getAHT10Humidity();

#endif