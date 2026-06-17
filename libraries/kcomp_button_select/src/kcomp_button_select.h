#ifndef _KCOMP_BUTTON_SELECT_H
#define _KCOMP_BUTTON_SELECT_H

#include "Arduino.h"
#include "kcomp_button.h"

#define BUTTON_SELECT_PIN 4


void initButtonSelect();

bool isButtonSelectPressed();

byte _debounceButtonSelect(const bool);

void _resetButtonSelect();

void attachButtonSelectClick(_buttonCallbackFunction);
void attachButtonSelectDoubleClick(_buttonCallbackFunction);
void attachButtonSelectLongPressed(_buttonCallbackFunction);
void attachButtonSelectLongReleased(_buttonCallbackFunction);

void pollSelectButton();

void _runButtonSelectFSM(bool);

#endif