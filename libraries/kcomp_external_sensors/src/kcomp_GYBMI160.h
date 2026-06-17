/*
  kcomp_BMI160.h - Library for GY-BMI160 (Bosch BMI160 IMU)
  
  Features:
  - Reads Accelerometer (X, Y, Z) in m/s^2
  - Reads Gyroscope (X, Y, Z) in deg/s
  - Calculates Pitch (Angle X) and Roll (Angle Y) using Trigonometry (Stable)
  - Calculates Yaw (Angle Z) using Gyro Integration (Subject to Drift)
*/

#ifndef _KCOMP_BMI160_H
#define _KCOMP_BMI160_H

#include "Arduino.h"

// --- I2C Addresses ---
// Check your module! If SDO is floating/VCC -> 0x69. If GND -> 0x68.
#define BMI160_ADDR_DEFAULT  0x69 
#define BMI160_ADDR_ALT      0x68 

// --- Data Structure ---
typedef struct {
    // --- Raw Sensor Data ---
    float accelX, accelY, accelZ; // Acceleration in m/s^2
    float gyroX, gyroY, gyroZ;    // Rotation speed in deg/s
    
    // --- Calculated Angles ---
    float angleX; // Pitch: Tilted Forward/Back (Stable, uses Gravity)
    float angleY; // Roll:  Tilted Left/Right  (Stable, uses Gravity)
    float angleZ; // Yaw:   Rotated Left/Right (Drifts over time!)
    
    // --- Misc ---
    float temperature; // Chip temperature in Celsius
} BMI160Data;

// --- Functions ---

/*
 * Initialize the BMI160.
 * Wakes up the sensor and sets ranges to ±2g and ±2000°/s.
 * Returns true if successful.
 */
bool initBMI160(uint8_t address = BMI160_ADDR_DEFAULT);

/*
 * Reads all sensors and performs angle calculations.
 * Call this in your loop() frequently.
 */
BMI160Data readBMI160(void);

/*
 * Resets the accumulated Z-Angle (Yaw) back to 0.
 * Useful for a "Recalibrate" button.
 */
void resetGyroZ(void);

#endif