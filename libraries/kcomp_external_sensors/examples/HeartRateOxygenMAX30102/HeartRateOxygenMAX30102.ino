#include "kcomp_MAX30102.h"

unsigned long last_print_time = 0;

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println("Initializing...");
    if (!initMAX30102()) {
        Serial.println("Sensor not found!");
        while(1);
    }
    Serial.println("Place finger on sensor.");
}

void loop() {
    // 1. CRITICAL: Call this as often as possible!
    // This keeps the math engine running.
    updateVitalSigns();

    // 3. Print Results (Every 1 second)
    // We don't want to spam the Serial Monitor, so we only print occasionally.
    if (millis() - last_print_time > 1000) {
        last_print_time = millis();

        int bpm = getHeartRate();
        int ox = getSPO2();

        if (bpm > 0) {
            Serial.print("Heart Rate: "); 
            Serial.print(bpm);
            Serial.print(" bpm \t Oxygen: "); 
            Serial.print(ox);
            Serial.println(" %");
        } else {
            Serial.println("No finger detected.");
        }
    }
}