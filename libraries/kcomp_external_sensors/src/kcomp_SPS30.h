#ifndef _KCOMP_SPS30_H
#define _KCOMP_SPS30_H

#include "Arduino.h"

#define SPS30_I2CADDR_DEFAULT 0x69   

// --- Enums for Smart Analysis ---
typedef enum {
    SPS30_AQI_GOOD,
    SPS30_AQI_MODERATE,
    SPS30_AQI_UNHEALTHY_SENSITIVE,
    SPS30_AQI_UNHEALTHY,
    SPS30_AQI_VERY_UNHEALTHY,
    SPS30_AQI_HAZARDOUS
} SPS30_AQI_t;

typedef enum {
    SPS30_SOURCE_CLEAN,       
    SPS30_SOURCE_COMBUSTION,  
    SPS30_SOURCE_DUST,        
    SPS30_SOURCE_MIXED        
} SPS30_Source_t;

// --- Data Structure ---
typedef struct {
    bool is_valid; // true if a new measurement was successfully read
    
    // Mass Concentration (ug/m3)
    uint16_t pm1_0;
    uint16_t pm2_5;
    uint16_t pm4_0;
    uint16_t pm10_0;

    // Number Concentration (Particles / cm3)
    uint16_t nc0_5;
    uint16_t nc1_0;
    uint16_t nc2_5;
    uint16_t nc4_0;
    uint16_t nc10_0;

    // Advanced
    float typical_particle_size; // in micrometers (um)
} sps30_data_t;


// --- Core Function Prototypes ---

bool initSPS30();

/*
 * Reads the latest measurements from the sensor.
 * Returns an sps30_data_t struct. Check the .is_valid flag to see if data is new!
 */
sps30_data_t readSPS30();


// --- Getters: Smart Analysis (No parameters needed!) ---
SPS30_AQI_t getSPS30_AirQualityLevel();
SPS30_Source_t getSPS30_PollutionSource();
uint8_t getSPS30_AirQualityScore(); 

#endif