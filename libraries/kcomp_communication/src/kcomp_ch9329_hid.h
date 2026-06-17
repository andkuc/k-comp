/*
  kcomp_CH9329_hid.h - Flat C-Style Library for CH9329
  Features: Mouse, Keyboard (Strings & Modifiers), Configuration
*/
#ifndef _KCOMP_CH9329_HID_H
#define _KCOMP_CH9329_HID_H

#include "Arduino.h"

// --- CONFIGURATION ---
#define CH9329_BAUD 9600 

// --- MODIFIERS ---
#define KEY_LEFT_CTRL     0x80
#define KEY_LEFT_SHIFT    0x81
#define KEY_LEFT_ALT      0x82
#define KEY_LEFT_GUI      0x83
#define KEY_RIGHT_CTRL    0x84
#define KEY_RIGHT_SHIFT   0x85
#define KEY_RIGHT_ALT     0x86
#define KEY_RIGHT_GUI     0x87

// --- MISC KEYS ---
#define KEY_UP_ARROW      0xDA
#define KEY_DOWN_ARROW    0xD9
#define KEY_LEFT_ARROW    0xD8
#define KEY_RIGHT_ARROW   0xD7
#define KEY_BACKSPACE     0xB2
#define KEY_TAB           0xB3
#define KEY_RETURN        0xB0
#define KEY_MENU          0xED 
#define KEY_ESC           0xB1
#define KEY_INSERT        0xD1
#define KEY_DELETE        0xD4
#define KEY_PAGE_UP       0xD3
#define KEY_PAGE_DOWN     0xD6
#define KEY_HOME          0xD2
#define KEY_END           0xD5
#define KEY_CAPS_LOCK     0xC1
#define KEY_PRINT_SCREEN  0xCE 
#define KEY_SCROLL_LOCK   0xCF
#define KEY_PAUSE         0xD0 

// --- NUMERIC KEYPAD ---
#define KEY_NUM_LOCK      0xDB
#define KEY_KP_SLASH      0xDC
#define KEY_KP_ASTERISK   0xDD
#define KEY_KP_MINUS      0xDE
#define KEY_KP_PLUS       0xDF
#define KEY_KP_ENTER      0xE0
#define KEY_KP_1          0xE1
#define KEY_KP_2          0xE2
#define KEY_KP_3          0xE3
#define KEY_KP_4          0xE4
#define KEY_KP_5          0xE5
#define KEY_KP_6          0xE6
#define KEY_KP_7          0xE7
#define KEY_KP_8          0xE8
#define KEY_KP_9          0xE9
#define KEY_KP_0          0xEA
#define KEY_KP_DOT        0xEB

// --- FUNCTION KEYS ---
#define KEY_F1            0xC2
#define KEY_F2            0xC3
#define KEY_F3            0xC4
#define KEY_F4            0xC5
#define KEY_F5            0xC6
#define KEY_F6            0xC7
#define KEY_F7            0xC8
#define KEY_F8            0xC9
#define KEY_F9            0xCA
#define KEY_F10           0xCB
#define KEY_F11           0xCC
#define KEY_F12           0xCD
#define KEY_F13           0xF0
#define KEY_F14           0xF1
#define KEY_F15           0xF2
#define KEY_F16           0xF3
#define KEY_F17           0xF4
#define KEY_F18           0xF5
#define KEY_F19           0xF6
#define KEY_F20           0xF7
#define KEY_F21           0xF8
#define KEY_F22           0xF9
#define KEY_F23           0xFA
#define KEY_F24           0xFB

// --- FUNCTION PROTOTYPES ---

// Setup & Configuration
void beginCH9329(int rxPin, int txPin);
void configureMode0(); // Force "Unbrick" to Mouse+KB mode

// Mouse Control
void moveMouse(int8_t x, int8_t y);
void clickMouse(uint8_t buttons); // 1=Left, 2=Right, 4=Middle
void scrollMouse(int8_t wheel);
void moveMouseAbsolute(uint16_t x, uint16_t y); // 0-4095

// Keyboard Control
size_t pressKey(uint8_t k);   // Press and hold
size_t releaseKey(uint8_t k); // Release specific key
void releaseAll();            // Release everything

// High Level Typing
size_t writeKey(uint8_t k);      // Click single key
void typeText(const char* text); // Type full string

#endif