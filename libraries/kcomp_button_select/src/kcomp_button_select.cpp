#include "kcomp_button_select.h"

_buttonCallbackFunction _buttonSelectClickFunction = NULL;
_buttonCallbackFunction _buttonSelectDoubleClickFunction = NULL;
_buttonCallbackFunction _buttonSelectLongPressStartFunction = NULL;
_buttonCallbackFunction _buttonSelectLongPressReleasedFunction = NULL;

_buttonState _buttonSelectState = BUTTON_INIT;

byte _buttonSelectDebouncedPinLevel = 0;
byte _buttonSelectLastDebouncePinLevel = 0;

unsigned long _now_select = 0;

unsigned long _buttonSelectLastDebounceTime = 0;
unsigned long _buttonSelectStartTime = 0; 

byte _buttonSelectClicks = 0;

void initButtonSelect() {
    pinMode(BUTTON_SELECT_PIN, INPUT);
}

bool isButtonSelectPressed() {
    return digitalRead(BUTTON_SELECT_PIN);
}

void _resetButtonSelect() {
  _buttonSelectState = BUTTON_INIT;
  _buttonSelectStartTime = 0;
  _buttonSelectClicks = 0;
  _buttonSelectDebouncedPinLevel = 0;
  _buttonSelectLastDebouncePinLevel = 0;
}

byte _debounceButtonSelect(const bool value) {
    _now_select = millis();
    if (_buttonSelectLastDebouncePinLevel == value) {
        if (_now_select - _buttonSelectLastDebounceTime >= _BUTTON_DEFAULT_DEBOUNCE_TIME) {
            _buttonSelectDebouncedPinLevel = value;
        }
    } else {
        _buttonSelectLastDebounceTime = _now_select;
        _buttonSelectLastDebouncePinLevel = value;
    }
    return _buttonSelectDebouncedPinLevel;
}

void attachButtonSelectClick(_buttonCallbackFunction function) {
      _buttonSelectClickFunction = function;
}

void attachButtonSelectDoubleClick(_buttonCallbackFunction function) {
      _buttonSelectDoubleClickFunction = function;
}

void attachButtonSelectLongPressed(_buttonCallbackFunction function) {
      _buttonSelectLongPressStartFunction = function;
}
void attachButtonSelectLongReleased(_buttonCallbackFunction function) {
      _buttonSelectLongPressReleasedFunction = function;
}

void pollSelectButton() {
     _runButtonSelectFSM(_debounceButtonSelect(isButtonSelectPressed()));
}

void _runButtonSelectFSM(bool isPressed) {
    unsigned long waitTime = (_now_select - _buttonSelectStartTime);

    switch(_buttonSelectState) {
        case BUTTON_INIT:
            if(isPressed) {
              _buttonSelectState = BUTTON_DOWN;
              _buttonSelectStartTime = _now_select;
              _buttonSelectClicks = 0;
            }  
            break;
        case BUTTON_DOWN: 
            if(!isPressed) {
                _buttonSelectState = BUTTON_UP;
                _buttonSelectStartTime = _now_select;
            } else if(isPressed && waitTime > _BUTTON_LONGLICK_TIME) {
                if(_buttonSelectLongPressStartFunction) {
                     _buttonSelectLongPressStartFunction();
                     _buttonSelectState = BUTTON_PRESSED;
                }
            }
            break;
        case BUTTON_UP:
            _buttonSelectClicks++;
            _buttonSelectState = BUTTON_COUNT;
            break;
        case BUTTON_COUNT:
            if(isPressed) {
                _buttonSelectState = BUTTON_DOWN;
                _buttonSelectStartTime = _now_select;
            } else if (waitTime >= _BUTTON_CLICK_TIME || _buttonSelectClicks == 2) {
                 if(_buttonSelectClicks == 1) {
                     if(_buttonSelectClickFunction) {
                         _buttonSelectClickFunction();
                     }
                 }
                 else if(_buttonSelectClicks == 2) {
                     if(_buttonSelectDoubleClickFunction) {
                         _buttonSelectDoubleClickFunction();
                     }
                 }
                 _resetButtonSelect();
            }
            break;
        case BUTTON_PRESSED: 
            if(!isPressed) {
              _buttonSelectState = BUTTON_RELEASED;
              _buttonSelectStartTime = _now_select;
            }
            break;
        case BUTTON_RELEASED: 
            if(_buttonSelectLongPressReleasedFunction) {
                _buttonSelectLongPressReleasedFunction();
            }
            _resetButtonSelect();
            break;
        default: 
          _buttonSelectState = BUTTON_INIT;
    }
}