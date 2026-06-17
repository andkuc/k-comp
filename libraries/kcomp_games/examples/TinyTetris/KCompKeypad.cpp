/* 
 * 
 * Dpad for Arduino TinyTetris 
 * 
 * Designed for https://github.com/AJRussell/Tiny-Tetris
 * 
 * Clumsy code by tobozo (c+) 2016
 * Apologies for coding horror
 * 
 * A. Kucher: Modified for KY-023
 */
 
#ifndef KCOMPKEYPAD
#define KCOMPKEYPAD

#include <Arduino.h>

#define JOYSTICK_LEFT 212
#define JOYSTICK_RIGHT 812
#define JOYSTICK_DOWN 212

static int dpadwarp[5] = { 0,0,0,0,0 };
static volatile int Debounce = 0;
static volatile bool processKey = true;
static volatile int currentPos;


class KCompKeypad
{
  static const int DebounceMax = 10;
  
  public:
    static int getPos() {
      if(digitalRead(4) == HIGH) {
        delay(200);
        return 0;
      }
      if(digitalRead(3) == HIGH) {
        delay(200);
        return 4;
      }
      else {
        int yval = analogRead(A2);
        int xval = analogRead(A1);
          
        if(yval < JOYSTICK_LEFT) {
          delay(100);
          return 1;
        }
        else if(yval > JOYSTICK_RIGHT) {
          delay(100);
          return 2;
        }
        else if(xval < JOYSTICK_DOWN) {
          delay(100);
          return 3;
        }
        else return -1;
      }

      return -1;
    }

    static boolean DoDebounce() {
      Debounce++;
      if(Debounce > DebounceMax) {
        return true;
      }
      return false;
    }

    static int setAccel(int acceleration, int offset) {
      if(processKey) {
        dpadwarp[currentPos] = millis();
      }
      if(millis() < dpadwarp[currentPos] + offset) {
        processKey = false;
      } else {
        processKey = true;
        acceleration = acceleration + 70;
        if (acceleration > offset) {
          acceleration = offset;
        }
      }
      return acceleration;
    }

  private:
     static int getPosition(int pin) {
      return analogRead(pin);
    }
};

#endif