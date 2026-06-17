#ifndef _KCOMP_AS608_H
#define _KCOMP_AS608_H

#include "Arduino.h"

// --- Standardized Returns ---
#define AS608_OK             0
#define AS608_NO_FINGER     -1
#define AS608_IMAGE_FAIL    -2
#define AS608_MATCH_FAIL    -3
#define AS608_ENROLL_FAIL   -4
#define AS608_COMM_ERROR    -5

// --- LED Control Constants ---
#define LED_BREATHING       1
#define LED_FLASHING        2
#define LED_ALWAYS_ON       3
#define LED_ALWAYS_OFF      4
#define LED_GRADUAL_ON      5
#define LED_GRADUAL_OFF     6

#define LED_RED             1
#define LED_BLUE            2
#define LED_PURPLE          3

// --- Core Functions ---
bool initAS608(uint8_t rxPin, uint8_t txPin, uint32_t baudRate);
bool checkAS608();
int16_t scanFingerprint();
int8_t enrollFingerprint(uint8_t target_id);
bool clearAllFingerprints();

// --- Advanced Database Management ---
int8_t deleteFingerprint(uint16_t id);
int16_t getFingerprintCount();
bool isSlotOccupied(uint16_t id);

// --- Advanced Hardware Control ---
void controlLED(uint8_t mode, uint8_t color, uint8_t speed, uint8_t cycles);

// --- Template Extraction ---
// Buffer must be exactly 512 bytes large!
bool downloadTemplate(uint16_t id, uint8_t* buffer);

#endif