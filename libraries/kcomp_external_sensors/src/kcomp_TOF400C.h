#ifndef _KCOMP_TOF400C_H
#define _KCOMP_TOF400C_H

#include "Arduino.h"

// --- Enums for Configuration ---
typedef enum {
    TOF400C_MODE_SHORT,   // Up to 1.3m (Best for bright ambient light)
    TOF400C_MODE_MEDIUM,  // Up to 3.0m
    TOF400C_MODE_LONG     // Up to 4.0m (Default, best in dim/indoor light)
} TOF400C_Mode_t;

// --- Data Structure ---
typedef struct {
    bool is_valid;        // true if a new measurement was successfully read
    uint16_t distance_mm; // Distance to target in millimeters
} tof400c_data_t;

// --- Core Function Prototypes ---

/*
 * Initializes the TOF400C (VL53L1X) sensor.
 * Sets up I2C, checks the device ID, boots the firmware, and applies tuning parms.
 * Returns true if successful, false if the sensor cannot be found or fails to boot.
 */
bool initTOF400C();

/*
 * Changes the maximum ranging mode. 
 * @param mode: TOF400C_MODE_SHORT, TOF400C_MODE_MEDIUM, or TOF400C_MODE_LONG
 */
void setTOF400CMode(TOF400C_Mode_t mode);

/*
 * Reads the latest measurement from the sensor.
 * Returns a tof400c_data_t struct. Always check .is_valid before using the distance!
 */
tof400c_data_t readTOF400C();

#endif