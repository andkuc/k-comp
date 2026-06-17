#ifndef _KCOMP_MAX30102_H
#define _KCOMP_MAX30102_H

#include "Arduino.h"


// --- Definitions ---
#define MAX30102_I2CADDR_DEFAULT  0x57

// --- Configuration ---
// Students can adjust these if readings are unstable
#define MAX_SAMPLES_BUFFER      5     // Smoothing factor for Heart Rate
#define PULSE_MIN_THRESHOLD     100   // IR signal strength change to count as a beat
#define PULSE_MAX_THRESHOLD     2000  // Upper limit to ignore noise/movement
#define PULSE_MIN_DELAY         300   // Min ms between beats (limit to 200 BPM)

// --- Function Prototypes ---

/*
 * Initialize the sensor.
 * Configures it for continuous reading.
 * Returns true if successful.
 */
bool initMAX30102(void);

/*
 * The "Worker" function.
 * CALL THIS IN YOUR LOOP CONSTANTLY!
 * It reads data from the sensor, processes the math, and updates the
 * internal heart rate and SpO2 variables.
 */
void updateVitalSigns(void);

/*
 * Returns the current calculated Heart Rate (BPM).
 * Returns 0 if no finger is detected.
 */
int getHeartRate(void);

/*
 * Returns the current Oxygen Saturation (%).
 */
int getSPO2(void);

/*
 * Reset the sensor.
 */
void resetMAX30102(void);

#endif