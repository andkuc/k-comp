#ifndef _KCOMP_VIBRATION_H
#define _KCOMP_VIBRATION_H

#include "Arduino.h"

// ==============================================================================
//  SETUP ROUTINE
// ==============================================================================

/*
 * Initializes the vibration motor. 
 * Must be connected to a PWM-capable digital pin (e.g., 3, 5, 6, 9, 10, 11).
 */
void initVibration(uint8_t pin);

// ==============================================================================
//  DIRECT HARDWARE CONTROL
// ==============================================================================

/*
 * Directly sets the vibration intensity and leaves it running.
 * intensity: 0 (Off) to 255 (Maximum Power)
 */
void setVibration(uint8_t intensity);

/*
 * Completely stops the motor. Equivalent to setVibration(0).
 */
void stopVibration();

// ==============================================================================
//  NON-BLOCKING HAPTIC FEEDBACK
// ==============================================================================

/*
 * Triggers a vibration pulse for a specific amount of time.
 * Does NOT block the CPU. The motor will run in the background.
 * intensity: 0 to 255
 * durationMs: Time in milliseconds (e.g., 200 for a short haptic click)
 */
void vibratePulseNonBlocking(uint8_t intensity, uint16_t durationMs);

/*
 * The Worker. Must be called constantly inside loop() if you are using
 * vibratePulseNonBlocking(). It checks the clock and turns the motor off
 * when the duration has expired.
 */
void pollVibration();

#endif