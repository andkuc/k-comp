#include "kcomp_button_Start.h"

_buttonCallbackFunction _buttonStartClickFunction = NULL;
_buttonCallbackFunction _buttonStartDoubleClickFunction = NULL;
_buttonCallbackFunction _buttonStartLongPressStartFunction = NULL;
_buttonCallbackFunction _buttonStartLongPressReleasedFunction = NULL;

_buttonState _buttonStartState = BUTTON_INIT;

byte _buttonStartDebouncedPinLevel = 0;
byte _buttonStartLastDebouncePinLevel = 0;

unsigned long _now_start = 0;

unsigned long _buttonStartLastDebounceTime = 0;
unsigned long _buttonStartStartTime = 0; 

byte _buttonStartClicks = 0;

void initButtonStart() {
    pinMode(BUTTON_START_PIN, INPUT);
}

bool isButtonStartPressed() {
    return digitalRead(BUTTON_START_PIN);
}

void _resetButtonStart() {
  _buttonStartState = BUTTON_INIT;
  _buttonStartStartTime = 0;
  _buttonStartClicks = 0;
  _buttonStartDebouncedPinLevel = 0;
  _buttonStartLastDebouncePinLevel = 0;
}

byte _debounceButtonStart(const bool value) {
    _now_start = millis();
    if (_buttonStartLastDebouncePinLevel == value) {
        if (_now_start - _buttonStartLastDebounceTime >= _BUTTON_DEFAULT_DEBOUNCE_TIME) {
            _buttonStartDebouncedPinLevel = value;
        }
    } else {
        _buttonStartLastDebounceTime = _now_start;
        _buttonStartLastDebouncePinLevel = value;
    }
    return _buttonStartDebouncedPinLevel;
}

void attachButtonStartClick(_buttonCallbackFunction function) {
      _buttonStartClickFunction = function;
}

void attachButtonStartDoubleClick(_buttonCallbackFunction function) {
      _buttonStartDoubleClickFunction = function;
}

void attachButtonStartLongPressed(_buttonCallbackFunction function) {
     _buttonStartLongPressStartFunction = function;
}

void attachButtonStartLongReleased(_buttonCallbackFunction function) {
      _buttonStartLongPressReleasedFunction = function;
}

void pollStartButton() {
     _runButtonStartFSM(_debounceButtonStart(isButtonStartPressed()));
}

void _runButtonStartFSM(bool isPressed) {
    unsigned long waitTime = (_now_start - _buttonStartStartTime);
    
	switch(_buttonStartState) {
        case BUTTON_INIT:
            if(isPressed) {
              _buttonStartState = BUTTON_DOWN;
              _buttonStartStartTime = _now_start;
              _buttonStartClicks = 0;
            }  
            break;
        case BUTTON_DOWN: 
            if(!isPressed) {
                _buttonStartState = BUTTON_UP;
                _buttonStartStartTime = _now_start;
            } else if(isPressed && waitTime > _BUTTON_LONGLICK_TIME) {
                if(_buttonStartLongPressStartFunction) {
                     _buttonStartLongPressStartFunction();
                     _buttonStartState = BUTTON_PRESSED;
                }
            }
            break;
        case BUTTON_UP:
            _buttonStartClicks++;
            _buttonStartState = BUTTON_COUNT;
            break;
        case BUTTON_COUNT:
            if(isPressed) {
                _buttonStartState = BUTTON_DOWN;
                _buttonStartStartTime = _now_start;
            } else if (waitTime >= _BUTTON_CLICK_TIME || _buttonStartClicks == 2) {
                 if(_buttonStartClicks == 1) {
                     if(_buttonStartClickFunction) {
                         _buttonStartClickFunction();
                     }
                 }
                 else if(_buttonStartClicks == 2) {
                     if(_buttonStartDoubleClickFunction) {
                         _buttonStartDoubleClickFunction();
                     }
                 }
                 _resetButtonStart();
            }
            break;
        case BUTTON_PRESSED: 
            if(!isPressed) {
              _buttonStartState = BUTTON_RELEASED;
              _buttonStartStartTime = _now_start;
            }
            break;
        case BUTTON_RELEASED: 
            if(_buttonStartLongPressReleasedFunction) {
                _buttonStartLongPressReleasedFunction();
            }
            _resetButtonStart();
            break;
        default: 
          _buttonStartState = BUTTON_INIT;
    }
}