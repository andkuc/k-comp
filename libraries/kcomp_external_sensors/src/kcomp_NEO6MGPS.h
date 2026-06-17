/*
  kcomp_NEO6MGPS.h - Lightweight, Zero-Dependency GPS Library for KComp
  Updated: Adds Timezone support and Local Time calculation.
*/

#ifndef _KCOMP_NEO6MGPS_H
#define _KCOMP_NEO6MGPS_H

#include "Arduino.h"

// --- Enums ---
typedef enum {
    CARDINAL_N = 0, CARDINAL_NE, CARDINAL_E, CARDINAL_SE,
    CARDINAL_S,     CARDINAL_SW, CARDINAL_W, CARDINAL_NW,
    CARDINAL_UNKNOWN
} GPSCardinal;

// --- Data Structure ---
typedef struct {
    double latitude;      // Decimal Degrees
    double longitude;     // Decimal Degrees
    double altitude;      // Meters
    double speed;         // km/h
    double course;        // Degrees
    
    // Time & Date (LOCAL TIME if offset is set, otherwise UTC)
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    
    // Timezone Info
    int timezone_offset;  // The offset used (e.g., 1 for UTC+1)
    bool dst_active;      // True if DST (+1h) was applied
    
    // Status
    uint32_t satellites;  
    double hdop;          
    bool valid;           
    bool updated;         
} GPSData;

// --- Core Functions ---

bool initNEO6MGPS(int rxPin, int txPin, long baud);
bool updateGPS(void);
GPSData getGPSData(void);

// --- Timezone Configuration ---

/*
 * Set the Timezone offset manually (in hours).
 * Example: Austria is UTC+1 (Winter), so use 1.
 */
void setGPSOffset(int hours);

/*
 * Enable/Disable Daylight Saving Time.
 * If true, adds an additional +1 hour to the time.
 */
void setGPSDST(bool enabled);

/*
 * Automatically estimate the timezone based on Longitude.
 * Warning: This calculates "Solar Time" (15 deg = 1 hour).
 * It ignores political borders! Use with caution.
 */
void autoEstimateTimezone(bool enable);

// --- Helper Functions ---
double gps_distanceBetween(double lat1, double long1, double lat2, double long2);
double gps_courseTo(double lat1, double long1, double lat2, double long2);
GPSCardinal gps_cardinal(double course);

#endif