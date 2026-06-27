#ifndef _KCOMP_DISTANCE_H
#define _KCOMP_DISTANCE_H

#include "Arduino.h"

#define DISTANCE_TRIG_PIN 11
#define DISTANCE_ECHO_PIN 10

#define DISTANCE_CONVERSION_FACTOR 0.1715

// echo timeout in us - otherwise pulseIn blocks ~1s when nothing comes back. 50ms ~= 8m, plenty for the HC-SR04
#define DISTANCE_ECHO_TIMEOUT_US 50000UL

const int trigPin = 11;
const int echoPin = 10;

void initDistanceSensor();

int getDistanceMM(); 


#endif