#ifndef _KCOMP_DS18B20_H
#define _KCOMP_DS18B20_H

#include "Arduino.h"

// --- Data Structure ---
typedef struct {
    bool is_valid;        // true if a measurement is complete and the checksum passes
    float temperature_c;  // Temperature in degrees Celsius
} ds18b20_data_t;

// --- Core Function Prototypes ---

/*
 * Initializes the 1-Wire bus on the specified pin.
 * Returns true if the sensor responds to a reset pulse, false if missing.
 */
bool initDS18B20(uint8_t pin);

/*
 * Tells the sensor to wake up and start calculating the temperature.
 * The waterproof sensor takes up to 750 milliseconds to calculate the temperature.
 */
void requestDS18B20();

/*
 * Reads the data from the sensor.
 * Returns a ds18b20_data_t struct. .is_valid will be false if it is still calculating.
 */
ds18b20_data_t readDS18B20();

#endif