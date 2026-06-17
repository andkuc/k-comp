#ifndef _KCOMP_COLOR_H
#define _KCOMP_COLOR_H

#include "Arduino.h"

// ==============================================================================
//  INTEGRATION TIME SETTINGS
// ==============================================================================
#define COLOR_TIME_2_4MS  0xFF 
#define COLOR_TIME_24MS   0xF6 
#define COLOR_TIME_50MS   0xEB 
#define COLOR_TIME_101MS  0xD5 
#define COLOR_TIME_154MS  0xC0 
#define COLOR_TIME_700MS  0x00 

// ==============================================================================
//  MEMORY-OPTIMIZED COLOR DEFINITIONS (1 Byte instead of Strings)
// ==============================================================================
#define COLOR_UNKNOWN      0
#define COLOR_BLACK        1
#define COLOR_WHITE        2
#define COLOR_GREY         3
#define COLOR_RED          4
#define COLOR_DARK_RED     5
#define COLOR_GREEN        6
#define COLOR_FOREST_GREEN 7
#define COLOR_LIME_GREEN   8
#define COLOR_BLUE         9
#define COLOR_NAVY_BLUE    10
#define COLOR_LIGHT_BLUE   11
#define COLOR_YELLOW       12
#define COLOR_LIGHT_YELLOW 13
#define COLOR_ORANGE       14
#define COLOR_PINK         15
#define COLOR_BROWN        16

// ==============================================================================
//  SETUP ROUTINE
// ==============================================================================
bool initColorSensor();
void setColorIntegrationTime(uint8_t timeSetting);

// ==============================================================================
//  STATE MACHINE & CALIBRATION
// ==============================================================================
void pollColorSensor();

/*
 * Sets the hardware limits to catch pure black (noise) and pure white (glare)
 * before the algorithm attempts to calculate percentages.
 */
void calibrateColorEngine(uint16_t blackThreshold, uint16_t whiteThreshold);

// ==============================================================================
//  GETTERS & AI ENGINE
// ==============================================================================
uint16_t getRed();
uint16_t getGreen();
uint16_t getBlue();
uint16_t getClear(); 

uint16_t getLux();
uint16_t getColorTemperature();

/*
 * Runs the Nearest Neighbor algorithm and returns a 1-byte Color Definition
 * (e.g., returns COLOR_RED, which equals 4).
 */
uint8_t estimateColor();

/*
 * Translates the 1-byte Color Definition back into a human-readable string.
 * Used strictly for printing to the Serial Monitor.
 */
const char* getColorName(uint8_t colorCode);

#endif