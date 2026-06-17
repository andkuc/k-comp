#include "kcomp_NEO6MGPS.h"
#include <SoftwareSerial.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_NMEA_LEN 100
#define MAX_FIELDS 20

// --- Internal State ---
static SoftwareSerial *gpsSerial = NULL;
static GPSData internalData = {0};
static char gpsBuffer[MAX_NMEA_LEN];
static int bufferIdx = 0;

// Timezone State
static int user_offset_hours = 0;
static bool user_dst = false;
static bool auto_timezone = false;

// --- Helper: Date Math (Days in Month) ---
static uint8_t daysInMonth(uint16_t year, uint8_t month) {
    if (month == 2) {
        // Leap year check: Divisible by 4, unless divisible by 100 but not 400
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
            return 29;
        return 28;
    }
    if (month == 4 || month == 6 || month == 9 || month == 11)
        return 30;
    return 31;
}

// --- Helper: Apply Timezone to UTC ---
static void applyTimezoneLogic(GPSData *d) {
    int total_offset = user_offset_hours;
    
    // 1. Auto-Estimate Timezone from Longitude?
    if (auto_timezone && d->longitude != 0.0) {
        // 15 degrees longitude = 1 hour difference
        total_offset = (int)round(d->longitude / 15.0);
    }
    
    // 2. Add DST?
    if (user_dst) {
        total_offset += 1;
    }
    
    d->timezone_offset = total_offset;
    d->dst_active = user_dst;

    if (total_offset == 0) return; // Nothing to do

    // 3. Add Offset to Hour
    int h = (int)d->hour + total_offset;

    // 4. Handle Hour Rollover
    if (h >= 24) {
        h -= 24;
        d->day++;
        // Handle Day/Month Rollover
        if (d->day > daysInMonth(d->year, d->month)) {
            d->day = 1;
            d->month++;
            if (d->month > 12) {
                d->month = 1;
                d->year++;
            }
        }
    } 
    else if (h < 0) {
        h += 24;
        if (d->day > 1) {
            d->day--;
        } else {
            // Roll back to previous month
            if (d->month > 1) {
                d->month--;
            } else {
                d->month = 12;
                d->year--;
            }
            d->day = daysInMonth(d->year, d->month);
        }
    }
    
    d->hour = (uint8_t)h;
}

// --- Helper: Parsing ---
static double convertNMEACoord(const char *raw, const char *dir) {
    if (raw == NULL || *raw == 0) return 0.0;
    double val = atof(raw);
    int dd = (int)(val / 100);
    double mm = val - (dd * 100);
    double res = dd + (mm / 60.0);
    if (dir && (dir[0] == 'S' || dir[0] == 'W')) res = -res;
    return res;
}

static bool parseNMEASentence(char* sentence) {
    char* fields[MAX_FIELDS];
    int fieldCount = 0;
    char* start = sentence;
    fields[fieldCount++] = start;
    
    while (*start && fieldCount < MAX_FIELDS) {
        if (*start == ',' || *start == '*') {
            *start = '\0'; 
            fields[fieldCount++] = start + 1;
        }
        start++;
    }

    if (strncmp(fields[0], "$GPRMC", 6) == 0) {
        char* status = (fieldCount > 2) ? fields[2] : NULL;
        
        if (status && status[0] == 'A') {
            internalData.valid = true;
            internalData.updated = true;

            // UTC Time
            if (fieldCount > 1 && strlen(fields[1]) >= 6) {
                char tmp[3] = {0};
                strncpy(tmp, fields[1], 2); internalData.hour = atoi(tmp);
                strncpy(tmp, fields[1]+2, 2); internalData.minute = atoi(tmp);
                strncpy(tmp, fields[1]+4, 2); internalData.second = atoi(tmp);
            }
            // Position
            if (fieldCount > 4) internalData.latitude = convertNMEACoord(fields[3], fields[4]);
            if (fieldCount > 6) internalData.longitude = convertNMEACoord(fields[5], fields[6]);
            // Speed/Course
            if (fieldCount > 7) internalData.speed = atof(fields[7]) * 1.852; 
            if (fieldCount > 8) internalData.course = atof(fields[8]);
            // Date
            if (fieldCount > 9 && strlen(fields[9]) >= 6) {
                char tmp[3] = {0};
                strncpy(tmp, fields[9], 2); internalData.day = atoi(tmp);
                strncpy(tmp, fields[9]+2, 2); internalData.month = atoi(tmp);
                strncpy(tmp, fields[9]+4, 2); internalData.year = 2000 + atoi(tmp);
            }

            // --- CRITICAL: Apply Local Time Calculation ---
            applyTimezoneLogic(&internalData);

            return true;
        } else {
            internalData.valid = false; 
        }
    }
    else if (strncmp(fields[0], "$GPGGA", 6) == 0) {
        if (fieldCount > 7) internalData.satellites = atoi(fields[7]);
        if (fieldCount > 8) internalData.hdop = atof(fields[8]);
        if (fieldCount > 9) internalData.altitude = atof(fields[9]);
    }
    return false;
}

// --- Public Config ---

void setGPSOffset(int hours) {
    user_offset_hours = hours;
    auto_timezone = false; // Manual overrides auto
}

void setGPSDST(bool enabled) {
    user_dst = enabled;
}

void autoEstimateTimezone(bool enable) {
    auto_timezone = enable;
}

// --- Standard Functions ---

bool initNEO6MGPS(int rxPin, int txPin, long baud) {
    if (gpsSerial != NULL) delete gpsSerial;
    gpsSerial = new SoftwareSerial(rxPin, txPin);
    if (gpsSerial == NULL) return false;
    gpsSerial->begin(baud);
    bufferIdx = 0;
    internalData.valid = false;
    // Default: UTC
    user_offset_hours = 0;
    user_dst = false;
    auto_timezone = false;
    return true;
}

bool updateGPS(void) {
    if (!gpsSerial) return false;
    bool newData = false;
    internalData.updated = false;
    while (gpsSerial->available() > 0) {
        char c = gpsSerial->read();
        if (c == '$') bufferIdx = 0; 
        if (bufferIdx < MAX_NMEA_LEN - 1) gpsBuffer[bufferIdx++] = c;
        if (c == '\n') {
            gpsBuffer[bufferIdx] = '\0'; 
            if (parseNMEASentence(gpsBuffer)) newData = true;
            bufferIdx = 0;
        }
    }
    return newData;
}

GPSData getGPSData(void) {
    return internalData;
}

// --- Math Helpers ---
double gps_distanceBetween(double lat1, double long1, double lat2, double long2) {
    double deltaLat = radians(lat2 - lat1);
    double deltaLon = radians(long2 - long1);
    lat1 = radians(lat1); lat2 = radians(lat2);
    double a = sin(deltaLat / 2.0) * sin(deltaLat / 2.0) +
               cos(lat1) * cos(lat2) * sin(deltaLon / 2.0) * sin(deltaLon / 2.0);
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    return 6371000.0 * c;
}

double gps_courseTo(double lat1, double long1, double lat2, double long2) {
    double y = sin(radians(long2 - long1)) * cos(radians(lat2));
    double x = cos(radians(lat1)) * sin(radians(lat2)) -
               sin(radians(lat1)) * cos(radians(lat2)) * cos(radians(long2 - long1));
    double res = degrees(atan2(y, x));
    return (res < 0) ? res + 360.0 : res;
}

GPSCardinal gps_cardinal(double course) {
    if (course < 0) return CARDINAL_UNKNOWN;
    int idx = (int)((course + 22.5) / 45.0);
    return (GPSCardinal)(idx % 8);
}