#ifndef _KCOMP_LED_H
#define _KCOMP_LED_H

#include "Arduino.h"

#define LED_PIN 5

void initLed();

void ledOn(byte);
void ledOff();
void ledFadeIn(byte, int);
void ledFadeOut(byte, int);

#endif