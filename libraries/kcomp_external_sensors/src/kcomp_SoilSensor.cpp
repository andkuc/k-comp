#include "kcomp_SoilSensor.h"

// Static variables to hold the state of our single sensor
static uint8_t sensor_pin = A0;
static uint16_t dry_val = 800; // Default dry air value
static uint16_t wet_val = 300; // Default water value
static bool is_active = false;

void initSoilSensor(uint8_t pin) {
    sensor_pin = pin;
    pinMode(sensor_pin, INPUT);
    is_active = true;
}

void setSoilCalibration(uint16_t dry_value, uint16_t wet_value) {
    dry_val = dry_value;
    wet_val = wet_value;
}

uint16_t readSoilRaw() {
    if (!is_active) return 0;

    // Take a small average of 5 readings to smooth out electrical noise
    uint32_t total = 0;
    for (int i = 0; i < 5; i++) {
        total += analogRead(sensor_pin);
        delay(2);
    }
    
    return total / 5;
}

uint8_t readSoilMoisture() {
    if (!is_active) return 0;

    uint16_t raw = readSoilRaw();

    // Map the raw value to a 0-100 percentage.
    // Notice that the dry/wet variables are inverted! (Dry is high, Wet is low)
    long percent = map(raw, dry_val, wet_val, 0, 100);

    // If the dirt is completely bone dry or underwater, the raw reading might drift
    // slightly past our calibration values. Constrain forces it to stay between 0-100.
    percent = constrain(percent, 0, 100);

    return (uint8_t)percent;
}