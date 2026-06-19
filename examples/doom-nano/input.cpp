#include <Arduino.h>
#include "input.h"
#include "kcomp_joystick.h"
#include "kcomp_button_select.h"
#include "kcomp_button_start.h"

// State flags for K-Comp button callbacks
volatile bool _fire_clicked = false;
volatile bool _start_clicked = false;

// K-Comp Event Callbacks
void onSelectClicked() {
  _fire_clicked = true;
}

void onStartClicked() {
  _start_clicked = true;
}

// Setup the K-Comp Peripherals
void input_setup() {
  initJoystick();
  initButtonSelect();
  initButtonStart();
  
  // Attach event handlers
  attachButtonSelectClick(onSelectClicked);
  attachButtonStartClick(onStartClicked);
}

// Must be called continuously in the game loop to service K-Comp buttons
void pollInputs() {
  pollSelectButton();
  pollStartButton();
}

// Joystick Navigation (Using K-Comp Analog Thresholds)
bool input_left() {
  return getJoystickXValue() < -250;
}

bool input_right() {
  return getJoystickXValue() > 250;
}

bool input_up() {
  return getJoystickYValue() > 250;
}

bool input_down() {
  return getJoystickYValue() < -250;
}

// Button Handling (Latches true for exactly one frame when clicked)
bool input_fire() {
  if (_fire_clicked) {
    _fire_clicked = false;
    return true;
  }
  return false;
}

bool input_start() {
  if (_start_clicked) {
    _start_clicked = false;
    return true;
  }
  return false;
}