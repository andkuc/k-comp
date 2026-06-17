#include "kcomp_MAX30102.h"
#include <Wire.h>
#include <math.h>

// --- Registers ---
#define MAX30102_REG_FIFO_WR_PTR  0x04
#define MAX30102_REG_FIFO_RD_PTR  0x06
#define MAX30102_REG_FIFO_DATA    0x07
#define MAX30102_REG_MODE_CONFIG  0x09
#define MAX30102_REG_SPO2_CONFIG  0x0A
#define MAX30102_REG_LED1_PA      0x0C
#define MAX30102_REG_LED2_PA      0x0D

// --- Global State ---
static int current_bpm = 0;
static int current_spo2 = 0;

// Filters
static float ir_dc = 0;
static float red_dc = 0;

// Beat Detection State
static float ir_ac_max = 20;  // Dynamic range tracker
static float ir_ac_min = -20;
static bool beat_trigger = false;
static long last_beat_time = 0;

// SpO2 Calculation State
static float red_ac_sq_sum = 0;
static float ir_ac_sq_sum = 0;
static int sample_cnt = 0;
static float spo2_history[5] = {0}; // Buffer to smooth SpO2
static uint8_t spo2_idx = 0;

// Helper: Write Register
static void writeReg(uint8_t r, uint8_t v) {
    Wire.beginTransmission(MAX30102_I2CADDR_DEFAULT);
    Wire.write(r); Wire.write(v);
    Wire.endTransmission();
}

// Helper: Read Next Sample
static bool readNextSample(uint32_t *red, uint32_t *ir) {
    Wire.beginTransmission(MAX30102_I2CADDR_DEFAULT);
    Wire.write(MAX30102_REG_FIFO_RD_PTR);
    Wire.endTransmission(false);
    Wire.requestFrom(MAX30102_I2CADDR_DEFAULT, 1);
    uint8_t r_ptr = Wire.read();

    Wire.beginTransmission(MAX30102_I2CADDR_DEFAULT);
    Wire.write(MAX30102_REG_FIFO_WR_PTR);
    Wire.endTransmission(false);
    Wire.requestFrom(MAX30102_I2CADDR_DEFAULT, 1);
    uint8_t w_ptr = Wire.read();

    if (r_ptr == w_ptr) return false; // No new data

    Wire.beginTransmission(MAX30102_I2CADDR_DEFAULT);
    Wire.write(MAX30102_REG_FIFO_DATA);
    Wire.endTransmission(false);
    Wire.requestFrom(MAX30102_I2CADDR_DEFAULT, 6);
    
    uint8_t d[6];
    for(int i=0; i<6; i++) d[i] = Wire.read();
    
    *red = ((uint32_t)d[0] << 16 | (uint32_t)d[1] << 8 | (uint32_t)d[2]) & 0x03FFFF;
    *ir  = ((uint32_t)d[3] << 16 | (uint32_t)d[4] << 8 | (uint32_t)d[5]) & 0x03FFFF;
    return true;
}

// --- Public Init ---
bool initMAX30102(void) {
    Wire.begin();
    
    // Check ID
    Wire.beginTransmission(MAX30102_I2CADDR_DEFAULT);
    Wire.write(0xFF);
    Wire.endTransmission(false);
    Wire.requestFrom(MAX30102_I2CADDR_DEFAULT, 1);
    if (Wire.read() != 0x15) return false;

    // Reset
    writeReg(MAX30102_REG_MODE_CONFIG, 0x40); 
    delay(100);

    // Config: FIFO Rollover, Avg 4
    writeReg(0x08, 0x10 | 0x02); 
    // Mode: SpO2
    writeReg(MAX30102_REG_MODE_CONFIG, 0x03);
    // SpO2: 100Hz, 411us pulse, 4096nA
    writeReg(MAX30102_REG_SPO2_CONFIG, 0x27); 
    
    // LED Power (Increased for better signal)
    writeReg(MAX30102_REG_LED1_PA, 0x30); 
    writeReg(MAX30102_REG_LED2_PA, 0x30); 

    return true;
}

void resetMAX30102(void) {
    writeReg(MAX30102_REG_MODE_CONFIG, 0x40);
    ir_dc = 0; red_dc = 0;
    current_bpm = 0; current_spo2 = 0;
}

// --- Improved Non-Blocking Algorithm ---
void updateVitalSigns(void) {
    uint32_t raw_red, raw_ir;
    
    // 1. Read one sample
    if (!readNextSample(&raw_red, &raw_ir)) return;

    // 2. Finger Detection (Signal too weak?)
    if (raw_ir < 50000) {
        current_bpm = 0;
        current_spo2 = 0;
        ir_dc = 0; // Reset DC filters
        return; 
    }

    // 3. DC Removal (Exponential Moving Average)
    if (ir_dc == 0) ir_dc = raw_ir;
    if (red_dc == 0) red_dc = raw_red;

    ir_dc  = (0.95 * ir_dc) + (0.05 * raw_ir);
    red_dc = (0.95 * red_dc) + (0.05 * raw_red);

    float ir_ac = raw_ir - ir_dc;
    float red_ac = raw_red - red_dc;

    // Accumulate AC power for SpO2 (Root Mean Square method)
    red_ac_sq_sum += red_ac * red_ac;
    ir_ac_sq_sum += ir_ac * ir_ac;
    sample_cnt++;

    // 4. Dynamic Threshold Logic
    // Update min/max bounds slowly to adapt to signal strength
    ir_ac_max = (0.98 * ir_ac_max) + (0.02 * ir_ac);
    ir_ac_min = (0.98 * ir_ac_min) + (0.02 * ir_ac);

    // 5. Beat Detection (Zero Crossing)
    // We wait for the signal to drop below the midpoint (falling edge)
    float threshold = (ir_ac_max + ir_ac_min) / 2.0;

    if (ir_ac < threshold && !beat_trigger && ir_ac < -10) {
        beat_trigger = true; // Beat started (falling)
    }

    if (ir_ac > threshold && beat_trigger) {
        // Beat finished (rising edge crossed zero)
        beat_trigger = false;
        long now = millis();
        long delta = now - last_beat_time;

        // Valid Beat? (300ms = 200 BPM, 2000ms = 30 BPM)
        if (delta > 300 && delta < 2000) {
            
            // --- A. Heart Rate Calc ---
            int instant_bpm = 60000 / delta;
            if (current_bpm == 0) current_bpm = instant_bpm;
            current_bpm = (current_bpm * 0.8) + (instant_bpm * 0.2); // Smooth it

            // --- B. SpO2 Calc (End of Beat) ---
            // Calculate Ratio of RMS Amplitudes
            // R = (RMS_Red / DC_Red) / (RMS_IR / DC_IR)
            
            float red_rms = sqrt(red_ac_sq_sum / sample_cnt);
            float ir_rms  = sqrt(ir_ac_sq_sum / sample_cnt);
            
            float r_value = (red_rms / red_dc) / (ir_rms / ir_dc);
            
            // Standard Maxim Calibration for SpO2
            // SpO2 = -45.060 * R * R + 30.354 * R + 94.845
            float instant_spo2 = -45.060 * r_value * r_value + 30.354 * r_value + 94.845;

            // Constrain
            if (instant_spo2 > 100) instant_spo2 = 100;
            if (instant_spo2 < 70) instant_spo2 = 70;

            // Smooth SpO2 (Moving Average of last 5 beats)
            spo2_history[spo2_idx] = instant_spo2;
            spo2_idx = (spo2_idx + 1) % 5;
            
            float spo2_avg = 0;
            for (int i=0; i<5; i++) spo2_avg += spo2_history[i];
            current_spo2 = (int)(spo2_avg / 5.0);

            // Reset accumulators for next beat
            red_ac_sq_sum = 0;
            ir_ac_sq_sum = 0;
            sample_cnt = 0;
        }
        last_beat_time = now;
    }
}

int getHeartRate(void) { return current_bpm; }
int getSPO2(void) { return current_spo2; }