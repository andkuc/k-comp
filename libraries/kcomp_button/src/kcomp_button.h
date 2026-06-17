#ifndef _KCOMP_BUTTON_H
#define _KCOMP_BUTTON_H

#include "Arduino.h"

#define _BUTTON_CLICK_TIME 200
#define _BUTTON_LONGLICK_TIME 600

#define _BUTTON_DEFAULT_DEBOUNCE_TIME 25

extern "C" {
  typedef void (*_buttonCallbackFunction)(void);
}

enum _buttonState : byte {
    BUTTON_INIT,
    BUTTON_DOWN,
    BUTTON_UP,
    BUTTON_COUNT,
    BUTTON_PRESSED,
    BUTTON_RELEASED
};

#endif