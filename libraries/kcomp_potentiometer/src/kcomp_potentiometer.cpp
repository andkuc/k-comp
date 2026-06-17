#include "kcomp_potentiometer.h"

//_potentiometerCallbackFunction _potentiometerOnChangeFunction = NULL;

void initPotentiometer() {
    pinMode(POTENTIOMETER_PIN, INPUT);
}

int getPotentiometerRaw() {
    return _POTENTIOMETER_MAX_VAL - analogRead(POTENTIOMETER_PIN);
}

int getPotentiometerValue() {
  return 100 - (int)(analogRead(POTENTIOMETER_PIN) * 100L / _POTENTIOMETER_MAX_VAL);
}

/*
void pollPotentiometer()
   ... 
   if(_potentiometerOnChangeFunction) {
      _potentiometerOnChangeFunction()
   }
}
*/
//void attachPotentiometerChange(_potentiometerCallbackFunction) {
//    _potentiometerOnChangeFunction = _potentiometerCallbackFunction; 
//}