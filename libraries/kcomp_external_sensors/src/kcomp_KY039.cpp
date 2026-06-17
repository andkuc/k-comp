#include "kcomp_KY039.h"

#define SAMP_SIZ 5
#define RISE_THRESHOLD 4

static uint8_t sensor_pin = A0;
static bool is_active = false;

// Output caching
static heartbeat_data_t current_data = {false, 0, 0, 0};

// Non-blocking 20ms chunk variables
static unsigned long chunk_start = 0;
static uint32_t chunk_sum = 0;
static uint16_t chunk_count = 0;

// Johan Ha Slope Detection Variables
static float reads[SAMP_SIZ] = {0};
static float sum = 0;
static int ptr = 0;
static float before = 0;
static int rise_count = 0;
static bool rising = false;

// Timing history for BPM & HRV
static unsigned long last_beat_time = 0;
static uint16_t first = 0;
static uint16_t second = 0;
static uint16_t third = 0;

void initHeartbeat(uint8_t pin) {
    sensor_pin = pin;
    pinMode(sensor_pin, INPUT);
    is_active = true;
    chunk_start = millis();
}

heartbeat_data_t updateHeartbeat() {
    // Reset the trigger flag every single loop
    current_data.beat_detected = false; 

    if (!is_active) return current_data;

    unsigned long now = millis();
    
    // Accumulate analog readings as fast as the loop runs
    chunk_sum += analogRead(sensor_pin);
    chunk_count++;
    
    // Every 20ms, process the chunk (Cancels 50Hz lighting noise natively)
    if (now - chunk_start >= 20) {
        float reader = 0;
        if (chunk_count > 0) {
            reader = (float)chunk_sum / chunk_count;
        }
        
        // Reset chunk accumulators for the next 20ms cycle
        chunk_sum = 0;
        chunk_count = 0;
        chunk_start += 20; 
        
        // Push to rolling average array (Moving Average Filter)
        sum -= reads[ptr];
        sum += reader;
        reads[ptr] = reader;
        ptr = (ptr + 1) % SAMP_SIZ;
        
        float last = sum / SAMP_SIZ; // The smoothed value
        current_data.signal = (uint16_t)last; 
        
        // Peak detection based on continuous rising slope
        if (last > before) {
            rise_count++;
            
            // If the slope has risen consecutively enough times, trigger a beat!
            if (!rising && rise_count > RISE_THRESHOLD) {
                rising = true;
                current_data.beat_detected = true;
                
                unsigned long time_since_last = now - last_beat_time;
                last_beat_time = now;
                
                // Shift the interval history
                third = second;
                second = first;
                first = time_since_last;
                
                // Calculate BPM using a weighted average for extreme stability
                // (0.4 * newest) + (0.3 * middle) + (0.3 * oldest)
                if (third > 0) { 
                    float weighted_avg = (0.4 * first) + (0.3 * second) + (0.3 * third);
                    current_data.bpm = (uint8_t)(60000.0 / weighted_avg);
                    
                    // HRV is the absolute difference between the last two beats
                    current_data.hrv_ms = abs((int)first - (int)second);
                }
            }
        } else {
            // The curve is falling or flat. Reset the rising state.
            rising = false;
            rise_count = 0;
        }
        before = last;
    }
    
    return current_data;
}