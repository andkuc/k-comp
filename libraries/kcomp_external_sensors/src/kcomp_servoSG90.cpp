/*
  kcomp_Servo.cpp - Implementation
*/

#include "kcomp_servoSG90.h"
#include <Servo.h>

// --- Internal State ---
static Servo _internalServos[MAX_SERVOS];
static uint8_t _servoTypes[MAX_SERVOS];
static bool _servoActive[MAX_SERVOS] = {false}; 

// --- Public Functions ---

bool initServo(uint8_t id, int pin, uint8_t type) {
    if (id < 0 || id >= MAX_SERVOS) return false; 

    _servoTypes[id] = type;
    
    // Attach and set to safe neutral position immediately
    _internalServos[id].attach(pin);
    _servoActive[id] = true;

    stopServo(id);
    
    return true;
}

void setServoAngle(uint8_t id, int angle) {
    // 1. Safety Checks
    if (id >= MAX_SERVOS || !_servoActive[id]) return;
    
    // 2. Type Check: Ensure this is actually a 180 servo
    if (_servoTypes[id] != SERVO_TYPE_180) return; 

    // 3. Constrain to physical limits (0-180)
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    
    _internalServos[id].write(angle);
}

void setServoSpeed(uint8_t id, int speed) {
    // 1. Safety Checks
    if (id >= MAX_SERVOS || !_servoActive[id]) return;

    // 2. Type Check: Ensure this is actually a 360 servo
    if (_servoTypes[id] != SERVO_TYPE_360) return;

    // 3. Map Speed % (-100 to 100) to PWM (0 to 180)
    // -100% -> 0   (Max Reverse)
    //    0% -> 90  (Stop)
    //  100% -> 180 (Max Forward)
    int pwmValue = map(speed, -100, 100, 0, 180);
    
    // Safety clamp
    if (pwmValue < 0) pwmValue = 0;
    if (pwmValue > 180) pwmValue = 180;

    _internalServos[id].write(pwmValue);
}

void stopServo(uint8_t id) {
    if (id >= MAX_SERVOS || !_servoActive[id]) return;

    if (_servoTypes[id] == SERVO_TYPE_360) {
        _internalServos[id].write(90); // Stop signal for continuous
    }
    // For 180 types, we do nothing (they hold their last position)
}