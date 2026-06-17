/*
  kcomp_Servo.h - Library for SG90 Micro Servos
  Style: Flat C (Simple functional interface)
  Supports: Up to 4 Servos (IDs 0-3)
*/

#ifndef _KCOMP_SERVOSG90_H
#define _KCOMP_SERVOSG90_H

#include "Arduino.h"

// --- Constants ---
#define SERVO_TYPE_180  0  // Standard positional servo (Robot Arms)
#define SERVO_TYPE_360  1  // Continuous rotation servo (Wheels)

#define MAX_SERVOS      4  // Maximum number of servos supported

// --- Functions ---

/*
 * Initialize a specific servo.
 * @param id   : The servo ID (0 to 3).
 * @param pin  : The Arduino pin connected to the signal wire (orange).
 * @param type : SERVO_TYPE_180 or SERVO_TYPE_360.
 * Returns true if successful.
 */
bool initServo(uint8_t id, int pin, uint8_t type);

/*
 * Control a 180° Standard Servo.
 * @param id    : The servo ID.
 * @param angle : The target angle in degrees (0 to 180).
 * Note: If you try to use this on a 360° servo, it will be ignored to prevent errors.
 */
void setServoAngle(uint8_t id, int angle);

/*
 * Control a 360° Continuous Servo.
 * @param id    : The servo ID.
 * @param speed : The target speed in percent (-100 to 100).
 * 0 = Stop. Positive = Forward. Negative = Backward.
 * Note: If you try to use this on a 180° servo, it will be ignored.
 */
void setServoSpeed(uint8_t id, int speed);

/*
 * Stop the servo immediately.
 * - For 360°: Sets speed to 0.
 * - For 180°: No effect (servo holds position).
 */
void stopServo(uint8_t id);

#endif