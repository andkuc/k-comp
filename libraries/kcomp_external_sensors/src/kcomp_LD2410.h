#ifndef _KCOMP_LD2410_H
#define _KCOMP_LD2410_H

#include "Arduino.h"

// --- Function Prototypes ---

/*
 * Initialize the Radar Sensor.
 * @param rxPin : Arduino pin connected to Sensor TX (e.g., 6)
 * @param txPin : Arduino pin connected to Sensor RX (e.g., 7)
 * @param baud  : The baud rate to use (Recommended: 9600)
 * @return true if sensor is detected.
 */
bool initLD2410(uint8_t rxPin, uint8_t txPin, long baud);

/*
 * Process Radar Data.
 * MUST be called constantly in loop().
 * Handles dynamic packet lengths automatically.
 * Returns true if new data was updated.
 */
bool updateLD2410();

// --- Data Getters ---
bool isPresenceDetected();      // True if ANY target is found
bool isMoving();                // True if walking/running
bool isStationary();            // True if sitting/breathing

int getDetectionDistance();     // Distance to target in cm
int getMovingEnergy();          // Strength of movement (0-100)
int getStationaryEnergy();      // Strength of breathing (0-100)

/*
 * UTILITY: Change the sensor's internal baud rate.
 * Note: If using SoftwareSerial, this might fail if the current speed is too high.
 */
void setLD2410BaudRate(long newBaud);

#endif