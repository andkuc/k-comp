#ifndef _input_h
#define _input_h

void input_setup();
void pollInputs(); // New function to service K-Comp hardware callbacks

bool input_up();
bool input_down();
bool input_left();
bool input_right();

bool input_fire(); // Mapped to K-Comp SELECT
bool input_start(); // Mapped to K-Comp START

#endif