#ifndef _KCOMP_JOYSTICK_H
#define _KCOMP_JOYSTICK_H

#include "Arduino.h"

#define JOYSTICK_X_PIN A2
#define JOYSTICK_Y_PIN A1

#define _JOYSTICK_MAX_VAL 1023

#define _JOYSTICK_MAX_INTENSITY 512

#define _JOYSTICK_GAP 50

enum joystickDirection : byte {
    JOYSTICK_CENTER,
    JOYSTICK_LEFT,
    JOYSTICK_RIGHT,
    JOYSTICK_UP,
    JOYSTICK_DOWN,
    JOYSTICK_UP_LEFT,
    JOYSTICK_UP_RIGHT,
    JOYSTICK_DOWN_LEFT,
    JOYSTICK_DOWN_RIGHT,
    JOYSTICK_UNDEFINED
};

struct joystickState {
    joystickDirection direction;
    int intensity; 
};



/*
extern "C" {
  typedef void (*_potentiometerCallbackFunction)(int);
}
*/

void initJoystick();

int getJoystickXRaw();
int getJoystickXValue();

int getJoystickYRaw();
int getJoystickYValue();

joystickDirection getDirection();

joystickState getJoystickState();

#endif