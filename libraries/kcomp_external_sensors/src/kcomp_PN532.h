#ifndef _KCOMP_PN532_H
#define _KCOMP_PN532_H

#include "Arduino.h"

// --- Standardized Return Codes ---
#define PN532_OK             0
#define PN532_ERROR         -1
#define PN532_TIMEOUT       -2
#define PN532_NO_CARD       -3
#define PN532_AUTH_FAIL     -4 

// --- Supported Tag Architectures ---
#define CARD_UNKNOWN         0
#define CARD_MIFARE_CLASSIC  1
#define CARD_NTAG            2

// ==============================================================================
// CORE FUNCTIONS
// ==============================================================================

/*
 * Initializes the I2C bus.
 * THE ENGINEERING CHOICE:
 * - Pass a valid pin number (e.g., 4) to use Hardware Interrupts (Efficient).
 * - Pass -1 to force Polling Mode (The CPU will block and wait for the PN532).
 */
bool initPN532();

/*
 * Asks the PN532 to look for a tag. 
 * If found, the library stores the tag's details INTERNALLY.
 * Returns PN532_OK on success.
 */
int8_t scanForTag();

// ==============================================================================
// TAG DATA GETTERS (Reads from the library's internal memory)
// ==============================================================================

// Returns the architecture (e.g., CARD_MIFARE_CLASSIC or CARD_NTAG)
uint8_t getTagType();

// Returns how many bytes long the Serial Number is (usually 4 or 7)
uint8_t getUidLength();

/*
 * Copies the Serial Number of the last scanned tag into your array.
 * uidArray must be at least 7 bytes long! (e.g., uint8_t myUid[7];)
 */
void getUid(uint8_t uidArray[]);

// ==============================================================================
// UNIFIED MEMORY OPERATIONS (Operates on the last scanned tag)
// ==============================================================================

/*
 * Reads exactly 16 bytes from the currently locked tag into your array.
 * - For MIFARE: 'slot' is the Block Number.
 * - For NTAG: 'slot' is the starting Page Number (reads pages X, X+1, X+2, X+3).
 * bufferArray MUST be exactly 16 bytes large!
 */
int8_t readTagMemory(uint8_t slot, uint8_t bufferArray[]);

/*
 * Writes exactly 16 bytes to the currently locked tag.
 * - For MIFARE: 'slot' is the Block Number.
 * - For NTAG: 'slot' is the starting Page Number (writes 4 pages sequentially).
 * dataArray MUST be exactly 16 bytes large!
 */
int8_t writeTagMemory(uint8_t slot, uint8_t dataArray[]);

/*
 * Overrides the default MIFARE authentication key (FF FF FF FF FF FF).
 */
void setMifareKey(uint8_t keyArray[]);

#endif