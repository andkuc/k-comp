#ifndef _KCOMP_POTENTIOMETER_H
#define _KCOMP_POTENTIOMETER_H

#include "Arduino.h"

#define POTENTIOMETER_PIN A0

#define _POTENTIOMETER_MAX_VAL 1023
/*
extern "C" {
  typedef void (*_potentiometerCallbackFunction)(int);
}
*/

void initPotentiometer();

int getPotentiometerRaw();
int getPotentiometerValue();

//void pollPotentiometer();

//void attachPotentiometerChange(_potentiometerCallbackFunction);

#endif