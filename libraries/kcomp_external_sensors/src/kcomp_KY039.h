#ifndef _KCOMP_KY039_H
#define _KCOMP_KY039_H

#include "Arduino.h"

// --- Data Structure ---
typedef struct {
    bool beat_detected;   // Flashes 'true' for exactly one cycle when a beat hits
    uint8_t bpm;          // Current Beats Per Minute (averaged)
    uint16_t hrv_ms;      // Heart Rate Variability (fluctuation between beats in ms)
    uint16_t signal;      // The heavily filtered waveform (Perfect for the Serial Plotter!)
} heartbeat_data_t;

// --- Core Function Prototypes ---
void initHeartbeat(uint8_t pin);
heartbeat_data_t updateHeartbeat();

#endif