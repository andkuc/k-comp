/*
  kcomp_AS7343.h - Lightweight, Zero-Dependency AS7343 14-Channel Spectral Sensor Library
  Uses direct I2C communication and leverages the sensor's internal Auto-SMUX sequencer.
*/

#ifndef _KCOMP_AS7343_H
#define _KCOMP_AS7343_H

#include "Arduino.h"

// --- Data Structure ---
typedef struct {
    // Raw Spectral Channels (14 discrete bands)
    uint16_t f1_405nm; // Violet
    uint16_t f2_425nm; // Violet/Blue
    uint16_t fz_450nm; // Deep Blue
    uint16_t f3_475nm; // Blue/Cyan
    uint16_t f4_515nm; // Cyan/Green
    uint16_t f5_550nm; // Green
    uint16_t fy_555nm; // Yellow/Green
    uint16_t fxl_600nm;// Orange
    uint16_t f6_640nm; // Red/Orange
    uint16_t f7_690nm; // Red
    uint16_t f8_745nm; // Deep Red
    
    uint16_t clear;    // Unfiltered light
    uint16_t nir;      // Near Infrared (855nm)
    
    bool valid;
    bool updated;
} AS7343Data;

// --- Core Functions ---

// Initializes I2C and configures the 18-channel auto-sequencer. Returns true if found.
bool initAS7343(void); 

// Triggers the hardware to read all channels (takes ~150ms total). Returns true on success.
bool updateAS7343(void);

// Returns the latest data struct
AS7343Data getAS7343Data(void);

// --- Helpful Extra Features ---

// Turns the onboard white LED on or off (great for illuminating objects)
// current_mA can be roughly 4 to 254 mA.
void setAS7343LED(bool enable, uint8_t current_mA = 10);

// Set the sensor gain (0 to 10). Higher gain = better for dark environments.
void setAS7343Gain(uint8_t gain_level);

#endif