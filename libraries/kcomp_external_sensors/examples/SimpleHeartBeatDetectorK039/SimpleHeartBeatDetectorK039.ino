#include "kcomp_KY039.h"

// Connect the KY-039 'S' Pin to Analog 03
#define PULSE_PIN A3 

// Connect an LED to Pin 13 to flash with your heart

void setup() {
    Serial.begin(115200);
    
    initHeartbeat(PULSE_PIN);
    
    Serial.println("K-Comp EKG System Online");
    Serial.println("Please place finger LIGHTLY on the sensor...");
}

void loop() {
    // 1. Run the "DSP engine" at maximum speed
    heartbeat_data_t heart = updateHeartbeat();
    
    // 2. Only output data when the wave hits its peak!
    if (heart.beat_detected) {
        Serial.print("❤️ BEAT! | BPM: ");
        Serial.print(heart.bpm);
        Serial.print(" | HRV: ");
        Serial.print(heart.hrv_ms);
        Serial.println(" ms");
        
    } // Notice: There is absolutely NO delay() in this loop!
}