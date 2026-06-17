#ifndef _KCOMP_DISTANCE_H
#define _KCOMP_DISTANCE_H

#include "Arduino.h"

#define DISTANCE_TRIG_PIN 11
#define DISTANCE_ECHO_PIN 10

#define DISTANCE_CONVERSION_FACTOR 0.1715

const int trigPin = 11;
const int echoPin = 10;

void initDistanceSensor();

int getDistanceMM(); 


#endif