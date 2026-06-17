#ifndef _KCOMP_SOIL_SENSOR_H
#define _KCOMP_SOIL_SENSOR_H

#include "Arduino.h"

/*
 * Initializes the soil sensor on the specified analog pin (e.g., A0).
 */
void initSoilSensor(uint8_t pin);

/*
 * Calibrates the sensor mapping. 
 * @param dry_value: The raw reading when the sensor is completely dry in the air.
 * @param wet_value: The raw reading when submerged to the white line in water.
 */
void setSoilCalibration(uint16_t dry_value, uint16_t wet_value);

/*
 * Returns the raw 10-bit analog reading (0-1023) directly from the ADC.
 */
uint16_t readSoilRaw();

/*
 * Calculates and returns the calibrated moisture level (0-100%).
 */
uint8_t readSoilMoisture();

#endif