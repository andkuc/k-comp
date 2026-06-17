#ifndef _KCOMP_AS7331_H
#define _KCOMP_AS7331_H

#include "Arduino.h"

#define AS7331_I2CADDR_DEFAULT  0x74    

// --- Registers ---
#define AS7331_REG_OSR          0x00    
#define AS7331_REG_CREG1        0x06    
#define AS7331_REG_CREG2        0x07    
#define AS7331_REG_CREG3        0x08    

// UV Data Registers
#define AS7331_REG_UVA          0x02    
#define AS7331_REG_UVB          0x03    
#define AS7331_REG_UVC          0x04    

// --- Gain Settings for CREG1 ---
#define AS7331_GAIN_2048X       0x00
#define AS7331_GAIN_1024X       0x01
#define AS7331_GAIN_512X        0x02
#define AS7331_GAIN_256X        0x03
#define AS7331_GAIN_128X        0x04
#define AS7331_GAIN_64X         0x05
#define AS7331_GAIN_32X         0x06
#define AS7331_GAIN_16X         0x07
#define AS7331_GAIN_8X          0x08
#define AS7331_GAIN_4X          0x09
#define AS7331_GAIN_2X          0x0A
#define AS7331_GAIN_1X          0x0B

// --- Function Prototypes ---
bool initAS7331();
void setAS7331Gain(uint8_t gainReg);

// Get calculated Irradiance (uW/cm2)
float getAS7331_UVA();
float getAS7331_UVB();
float getAS7331_UVC();

// Get Raw ADC Counts (Great for testing if the sensor is alive!)
uint16_t getRawUVA();
uint16_t getRawUVB();
uint16_t getRawUVC();

#endif