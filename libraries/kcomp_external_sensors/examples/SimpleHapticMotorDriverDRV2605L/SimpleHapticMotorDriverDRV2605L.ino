#include "kcomp_DRV2605L.h"

void setup() {
  Serial.begin(115200);
  
  if (!initDRV2605L()) {
    Serial.println("Hardware Error: DRV2605L not found. Check wiring!");
    while(1); 
  }
  
  Serial.println("--- Tactile Interface Ready ---");
  Serial.println("1: Play 8-Slot Complex Sequence (Fire and Forget)");
  Serial.println("2: Enter Audio-to-Vibe Mode (Sensory Substitution)");
  Serial.println("3: Real-Time Playback (3-Second Custom Vibe)");
  Serial.println("0: Exit Audio Mode / Stop Motor");
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    while(Serial.available()) Serial.read(); // Clear buffer
    
    if (cmd == '1') {
      Serial.println(">> Playing 8-Slot Complex Sequence...");
      enableInternalROMMode(); // Ensure we are in standard code mode
      
      setHapticSequenceSlot(0, VIBE_SOFT_BUMP_100);    // 1. Gentle start
      setHapticSequenceSlot(1, VIBE_SHARP_TICK_100);   // 2. Quick tick
      setHapticSequenceSlot(2, VIBE_SHARP_TICK_100);   // 3. Quick tick
      setHapticSequenceSlot(3, VIBE_RAMP_UP_SHORT);    // 4. Build up power
      setHapticSequenceSlot(4, VIBE_STRONG_CLICK_100); // 5. The climax peak
      setHapticSequenceSlot(5, VIBE_RAMP_DOWN_SHORT);  // 6. Fade out
      setHapticSequenceSlot(6, VIBE_SOFT_FUZZ_60);     // 7. Lingering texture
      setHapticSequenceSlot(7, VIBE_STOP);             // 8. CRITICAL: End marker
      
      playHapticSequence();
    }
    
    else if (cmd == '2') {
      Serial.println(">> AUDIO MODE ACTIVE.");
      Serial.println("Send audio into the 'IN' pin. The motor will follow the beat.");
      enableAudioMode(); 
    }

    else if (cmd == '3') {
      Serial.println(">> Bypassing ROM: Starting 3-second vibration...");
      
      // 1. Switch the chip into Real-Time Mode
      enableRealTimeMode();
      
      // 2. Turn the motor on at 50% power (127 out of 255)
      setRealTimeIntensity(127);
      
      // 3. The Coprocessor is dumb now. You are responsible for the timing! 
      delay(3000); 
      
      // 4. Turn the motor off
      setRealTimeIntensity(0);
      
      // 5. Return the chip to standard ROM mode so '1' works again
      enableInternalROMMode(); 
      
      Serial.println(">> Done.");
    }
    
    else if (cmd == '0') {
      Serial.println(">> Stopped. Returned to standard mode.");
      enableInternalROMMode();
      stopHaptic();
    }
  }
}