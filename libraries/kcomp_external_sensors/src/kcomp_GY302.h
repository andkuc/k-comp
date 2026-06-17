/*
  kcomp_GY302.h - Library for GY-302 (BH1750)
  Features: Resolution Switching (Normal vs Fast vs High Precision).
*/

#ifndef _KCOMP_GY302_H
#define _KCOMP_GY302_H

#include "Arduino.h"

// --- I2C Addresses ---
#define GY302_ADDR_DEFAULT  0x23
#define GY302_ADDR_HIGH     0x5C

// --- Resolution Modes ---
// Mode 0: Normal (1 Lux precision, ~120ms measurement time)
// Mode 1: High Precision (0.5 Lux precision, ~120ms measurement time)
// Mode 2: Fast / Low Res (4 Lux precision, ~16ms measurement time)
typedef enum {
    GY302_MODE_NORMAL = 0x10,
    GY302_MODE_HIGH   = 0x11,
    GY302_MODE_FAST   = 0x13
} GY302Mode;

// --- Core Functions ---

/*
 * Initialize the GY-302 sensor.
 * Returns true if the sensor responded correctly.
 */
bool initGY302(uint8_t address = GY302_ADDR_DEFAULT);

/*
 * Read the current light level in Lux.
 * Returns -1.0 if the reading failed.
 */
float getGY302Lux(void);

// --- New Feature ---

/*
 * Change the resolution/speed of the sensor.
 * Use GY302_MODE_FAST for quick updates (e.g. gesture detection).
 * Use GY302_MODE_HIGH for dark environments.
 */
void setGY302Mode(GY302Mode mode);

#endif