#ifndef _KCOMP_MQSENSORS_H
#define _KCOMP_MQSENSORS_H

#include "Arduino.h"

// --- Supported Sensor Types ---
#define SENSOR_MQ2    2    // Flammable gas, Smoke
#define SENSOR_MQ3    3    // Alcohol vapor
#define SENSOR_MQ135  135  // Air Quality (VOCs)

// --- Default Clean Air Ratios (Sensitivity) ---
// Lower number = MORE sensitive to gas spikes
#define DEFAULT_RATIO_MQ2   5.0 
#define DEFAULT_RATIO_MQ3   5.0   // Pre-tuned for budget hardware
#define DEFAULT_RATIO_MQ135 3.6   

// --- Intensity Status Categories ---
#define STATUS_CLEAN       0   // 0% - 33% 
#define STATUS_MODERATE    1   // 34% - 66% 
#define STATUS_HIGH        2   // 67% - 100% 

// --- Core Function Prototypes ---

void initMQ(uint8_t pin, uint8_t sensor_type);

/*
 * OPTIONAL: Overrides the default sensitivity ratio. 
 * Must be called AFTER initMQ() but BEFORE calibrateMQ().
 * Lowering the ratio makes the sensor much more reactive.
 */
void tuneSensitivity(uint8_t pin, float custom_ratio);

/*
 * Run this ONCE in clean, fresh air.
 * Establishes the 0% baseline using the active sensitivity ratio.
 */
void calibrateMQ(uint8_t pin);

/*
 * Universal Intensity Score (0 to 100%).
 */
uint8_t readSensorScore(uint8_t pin);

/*
 * Returns a simple category (Clean, Moderate, High)
 */
uint8_t readSensorStatus(uint8_t pin);

#endif