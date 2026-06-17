#include "kcomp_joystick.h"

void initJoystick() {
    pinMode(JOYSTICK_X_PIN, INPUT);
    pinMode(JOYSTICK_Y_PIN, INPUT);
}

int getJoystickXRaw() {
    return analogRead(JOYSTICK_X_PIN);
}

int getJoystickXValue() {
    return getJoystickXRaw() - _JOYSTICK_MAX_VAL/2;
}

int getJoystickYRaw() {
    return analogRead(JOYSTICK_Y_PIN);
}
int getJoystickYValue() {
  return getJoystickYRaw() - _JOYSTICK_MAX_VAL/2;
}

joystickDirection getDirection() {
    joystickDirection direction;
    int x = getJoystickXValue();
    int y = getJoystickYValue();

    if (x >= -_JOYSTICK_GAP && x <= _JOYSTICK_GAP) {
        x = 0;
    }
    if (y >= -_JOYSTICK_GAP && y <= _JOYSTICK_GAP) {
        y = 0;
    }

    if (x == 0 && y == 0) {
        direction = JOYSTICK_CENTER;
    } else if (x < 0 && y == 0) {
        direction = JOYSTICK_LEFT;
    } else if (x > 0 && y == 0) {
        direction = JOYSTICK_RIGHT;
    } else if (x == 0 && y > 0) {
        direction = JOYSTICK_UP;
    } else if (x == 0 && y < 0) {
        direction = JOYSTICK_DOWN;
    } else if (x < 0 && y > 0) {
       direction = JOYSTICK_UP_LEFT;
    } else if (x > 0 && y > 0) {
       direction = JOYSTICK_UP_RIGHT;
    } else if (x < 0 && y < 0) {
        direction = JOYSTICK_DOWN_LEFT;
    } else if (x > 0 && y < 0) {
        direction = JOYSTICK_DOWN_RIGHT;
    } else {
        direction = JOYSTICK_UNDEFINED;
    }

    return direction;
} 

joystickState getJoystickState() {
    joystickState state; 
	long temp = 0;
	double s = 0.0;

    state.direction = getDirection();
  
    switch(state.direction) {
        case JOYSTICK_CENTER: 
            state.intensity = 0;
            break;
        case JOYSTICK_LEFT:
        case JOYSTICK_RIGHT:
            state.intensity = abs((int)(getJoystickXValue() * 100L / _JOYSTICK_MAX_INTENSITY));
            break;
        case JOYSTICK_UP:
        case JOYSTICK_DOWN:
            state.intensity = abs((int)(getJoystickYValue() * 100L / _JOYSTICK_MAX_INTENSITY));
            break;
        case JOYSTICK_UP_LEFT:
        case JOYSTICK_UP_RIGHT:
        case JOYSTICK_DOWN_LEFT:
        case JOYSTICK_DOWN_RIGHT:
            temp = (long)getJoystickXValue()*(long)getJoystickXValue() + (long)getJoystickYValue()*(long)getJoystickYValue();
            s = sqrt(temp);
            // some correction factor
            s = 95.0 * s / _JOYSTICK_MAX_INTENSITY;
            state.intensity = min((int)s, 100);
            break;
        default:
            state.direction = JOYSTICK_UNDEFINED;
            state.intensity = -1;
    }
    return state;  
}