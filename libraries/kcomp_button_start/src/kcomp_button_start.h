#ifndef _KCOMP_BUTTON_START_H
#define _KCOMP_BUTTON_START_H

#include "Arduino.h"
#include "kcomp_button.h"

#define BUTTON_START_PIN 3

void initButtonStart();

bool isButtonStartPressed();

byte _debounceButtonStart(const bool);

void _resetButtonStart();

void attachButtonStartClick(_buttonCallbackFunction);
void attachButtonStartDoubleClick(_buttonCallbackFunction);
void attachButtonStartLongPressed(_buttonCallbackFunction);
void attachButtonStartLongReleased(_buttonCallbackFunction);

void pollStartButton();

void _runButtonStartFSM(bool);

#endif