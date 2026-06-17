#include "kcomp_VibrationMotor.h"

// Hidden internal state variables
static uint8_t _vibPin = 255; 
static unsigned long _vibStartTime = 0;
static uint16_t _vibDuration = 0;
static bool _isPulsing = false;

void initVibration(uint8_t pin) {
    _vibPin = pin;
    pinMode(_vibPin, OUTPUT);
    analogWrite(_vibPin, 0); // Ensure it starts completely off
}

void setVibration(uint8_t intensity) {
    if (_vibPin == 255) return; // Safety check: Was init called?
    
    analogWrite(_vibPin, intensity);
    _isPulsing = false; // Override any running pulse
}

void stopVibration() {
    if (_vibPin == 255) return;
    
    analogWrite(_vibPin, 0);
    _isPulsing = false;
}

void vibratePulseNonBlocking(uint8_t intensity, uint16_t durationMs) {
    if (_vibPin == 255) return;
    
    analogWrite(_vibPin, intensity);
    _vibStartTime = millis();
    _vibDuration = durationMs;
    _isPulsing = true;
}

void pollVibration() {
    // If a pulse is currently active, check the clock
    if (_isPulsing) {
        if (millis() - _vibStartTime >= _vibDuration) {
            analogWrite(_vibPin, 0); // Turn off the motor
            _isPulsing = false;      // Reset the state machine
        }
    }
}