#ifndef _KCOMP_DRV2605L_H
#define _KCOMP_DRV2605L_H

#include "Arduino.h"

// ==============================================================================
// HAPTIC EFFECT DICTIONARY (Memory-Optimized Macros)
// The DRV2605L contains 117 total effects. Here is the comprehensive library.
// ==============================================================================

// --- CLICKS & BUMPS ---
#define VIBE_STOP                 0
#define VIBE_STRONG_CLICK_100     1
#define VIBE_STRONG_CLICK_60      2
#define VIBE_STRONG_CLICK_30      3
#define VIBE_SHARP_CLICK_100      4
#define VIBE_SHARP_CLICK_60       5
#define VIBE_SHARP_CLICK_30       6
#define VIBE_SHARP_TICK_100       7
#define VIBE_SHARP_TICK_60        8
#define VIBE_SHARP_TICK_30        9
#define VIBE_SOFT_BUMP_100        10
#define VIBE_SOFT_BUMP_60         11
#define VIBE_SOFT_BUMP_30         12
#define VIBE_DOUBLE_CLICK_100     13
#define VIBE_DOUBLE_CLICK_60      14
#define VIBE_TRIPLE_CLICK_100     15
#define VIBE_SOFT_FUZZ_60         16

// --- ALERTS & PULSES ---
#define VIBE_STRONG_BUZZ_100      47
#define VIBE_STRONG_BUZZ_80       48
#define VIBE_STRONG_BUZZ_60       49
#define VIBE_STRONG_BUZZ_40       50
#define VIBE_STRONG_BUZZ_20       51
#define VIBE_PULSE_ALERT_100      52
#define VIBE_PULSE_ALERT_80       53
#define VIBE_PULSE_ALERT_60       54
#define VIBE_PULSE_ALERT_40       55
#define VIBE_PULSE_ALERT_20       56

// --- TRANSITIONS & RAMPS ---
#define VIBE_TRANSITION_UP        58
#define VIBE_TRANSITION_DOWN      59
#define VIBE_RAMP_UP_LONG         114
#define VIBE_RAMP_DOWN_LONG       115
#define VIBE_RAMP_UP_SHORT        116
#define VIBE_RAMP_DOWN_SHORT      117

// --- SPECIAL EFFECTS ---
#define VIBE_HEARTBEAT            73
#define VIBE_HEARTBEAT_FAST       74
#define VIBE_HEARTBEAT_SLOW       75


// ==============================================================================
//  SETUP ROUTINE
// ==============================================================================
bool initDRV2605L();

// ==============================================================================
//  DIRECT COMMANDS (ROM Mode)
// ==============================================================================
void playHapticEffect(uint8_t effectID);
void stopHaptic();

// ==============================================================================
//  SEQUENCE BUILDER
// ==============================================================================
void setHapticSequenceSlot(uint8_t slotNumber, uint8_t effectID);
void playHapticSequence();

// ==============================================================================
//  AUDIO-TO-VIBE ENGINE (Sensory Substitution)
// ==============================================================================

/*
 * Switches the chip into Audio-to-Vibe mode. 
 * The chip stops listening to I2C sequence commands and instead listens 
 * to the analog audio signal physically wired to the 'IN' pin.
 */
void enableAudioMode();

/*
 * Switches the chip back to standard Internal ROM mode.
 * Use this to play normal clicks and sequences again.
 */
void enableInternalROMMode();

// ==============================================================================
//  REAL-TIME PLAYBACK (CUSTOM LENGTH)
// ==============================================================================

/*
 * Bypasses the internal memory and allows you to control the raw motor power.
 */
void enableRealTimeMode();

/*
 * Sets the raw vibration intensity in Real-Time Mode.
 * intensity: 0 (Off) to 255 (Max Power)
 */
void setRealTimeIntensity(uint8_t intensity);

#endif