#include "kcomp_MQSensors.h"

#define MAX_MQ_SENSORS 4

// The structure now holds a custom ratio for every single sensor
typedef struct {
    uint8_t pin;
    uint8_t type;
    float R0; 
    float current_ratio; // <--- The dynamic sensitivity dial
    bool is_active;
} mq_state_t;

static mq_state_t sensors[MAX_MQ_SENSORS] = {0};

// --- Private Helpers ---
static int getSensorIndex(uint8_t pin) {
    for (int i = 0; i < MAX_MQ_SENSORS; i++) {
        if (sensors[i].is_active && sensors[i].pin == pin) return i;
    }
    return -1;
}

static float getRsBase(uint8_t pin) {
    int adc = analogRead(pin);
    if (adc <= 0) return 999999.0;
    if (adc >= 1023) return 0.001; 
    
    return ((1023.0 / (float)adc) - 1.0);
}

// --- Public Functions ---

void initMQ(uint8_t pin, uint8_t sensor_type) {
    if (getSensorIndex(pin) != -1) return; 

    for (int i = 0; i < MAX_MQ_SENSORS; i++) {
        if (!sensors[i].is_active) {
            sensors[i].pin = pin;
            sensors[i].type = sensor_type;
            sensors[i].R0 = 1.0; 
            sensors[i].is_active = true;
            
            // Load the default ratios automatically on initialization
            if (sensor_type == SENSOR_MQ2)   sensors[i].current_ratio = DEFAULT_RATIO_MQ2;
            if (sensor_type == SENSOR_MQ3)   sensors[i].current_ratio = DEFAULT_RATIO_MQ3;
            if (sensor_type == SENSOR_MQ135) sensors[i].current_ratio = DEFAULT_RATIO_MQ135;
            
            pinMode(pin, INPUT);
            break;
        }
    }
}

void tuneSensitivity(uint8_t pin, float custom_ratio) {
    int idx = getSensorIndex(pin);
    if (idx != -1) {
        // Prevent math crashes (ratio cannot be exactly 1.0 or less)
        if (custom_ratio <= 1.0) custom_ratio = 1.01; 
        sensors[idx].current_ratio = custom_ratio;
    }
}

void calibrateMQ(uint8_t pin) {
    int idx = getSensorIndex(pin);
    if (idx == -1) return;

    float rs_sum = 0;
    for (int i = 0; i < 50; i++) {
        rs_sum += getRsBase(pin);
        delay(10);
    }
    float rs_avg = rs_sum / 50.0;

    // Set the baseline using the sensor's current dynamic ratio
    sensors[idx].R0 = rs_avg / sensors[idx].current_ratio;
}

uint8_t readSensorScore(uint8_t pin) {
    int idx = getSensorIndex(pin);
    if (idx == -1) return 0;

    float rs = getRsBase(pin);
    float ratio = rs / sensors[idx].R0;
    
    float baseline = sensors[idx].current_ratio;

    // Linear Map: Clean Air Ratio -> 0%, Saturated (Ratio <= 1.0) -> 100%
    float score = ((baseline - ratio) / (baseline - 1.0)) * 100.0;

    if (score < 0) score = 0;
    if (score > 100) score = 100;

    return (uint8_t)score;
}

uint8_t readSensorStatus(uint8_t pin) {
    uint8_t score = readSensorScore(pin);
    
    if (score < 33) return STATUS_CLEAN;
    if (score < 66) return STATUS_MODERATE;
    return STATUS_HIGH;
}