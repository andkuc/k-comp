#include "kcomp_distance.h"

void initDistanceSensor() {
    pinMode(DISTANCE_TRIG_PIN, OUTPUT);
    pinMode(DISTANCE_ECHO_PIN, INPUT);
}

int getDistanceMM() {
    digitalWrite(DISTANCE_TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(DISTANCE_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(DISTANCE_TRIG_PIN, LOW);

    return (int)(pulseIn(DISTANCE_ECHO_PIN, HIGH, DISTANCE_ECHO_TIMEOUT_US) * DISTANCE_CONVERSION_FACTOR);
  
}