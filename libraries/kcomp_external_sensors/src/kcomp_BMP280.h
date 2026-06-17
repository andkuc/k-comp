#ifndef _KCOMP_BMP280_H
#define _KCOMP_BMP280_H

#include "Arduino.h"

// --- Definitions ---
#define BMP280_I2CADDR_DEFAULT  0x76    ///< Common default address (Check your module: could be 0x77)
#define BMP280_CHIP_ID          0x58    ///< Default chip ID for BMP280

// --- Registers ---
#define BMP280_REG_TEMP_XLSB    0xFC
#define BMP280_REG_TEMP_LSB     0xFB
#define BMP280_REG_TEMP_MSB     0xFA
#define BMP280_REG_PRESS_XLSB   0xF9
#define BMP280_REG_PRESS_LSB    0xF8
#define BMP280_REG_PRESS_MSB    0xF7
#define BMP280_REG_CONFIG       0xF5
#define BMP280_REG_CTRL_MEAS    0xF4
#define BMP280_REG_STATUS       0xF3
#define BMP280_REG_RESET        0xE0
#define BMP280_REG_ID           0xD0
#define BMP280_REG_CALIB_START  0x88

// --- Function Prototypes ---

/*
 * Initialize the BMP280 sensor.
 * Starts I2C, reads calibration data, and sets default config.
 * Returns true if chip ID is correct, false otherwise.
 */
bool initBMP280();

/*
 * Read Temperature in Degrees Celsius.
 */
float getBMP280Temperature();

/*
 * Read Pressure in Pascals (Pa).
 * Divide by 100 to get hPa (hectopascals/millibars).
 */
float getBMP280Pressure();

/*
 * Calculate approximate altitude in meters.
 * @param seaLevelhPa : Pressure at sea level (default ~1013.25)
 */
float getBMP280Altitude(float seaLevelhPa = 1013.25);

#endif