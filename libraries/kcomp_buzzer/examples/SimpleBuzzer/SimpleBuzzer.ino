/*
  Program Title: Simple Buzzer Test Program
  Author: Andreas Kucher
  Date: April 29, 2025

  Program Summary:
  This program demonstrates how to initialize the buzzer
  on K-COMP, beep and play some melody (blocking and non-blocking).
  It uses kcomp_distance.h.
  Serial messages are printed to the Serial Monitor.
*/

// Include the header file that contains functions related to the
// buzzer.
#include <kcomp_buzzer.h>

// The setup() function runs once when KCOMP is powered on or reset
void setup() {
  // Start serial communication at 115200 baud for messages
  Serial.begin(115200);

  // Initialize the buzzer.
  initBuzzer();
  // Print a message to the Serial Monitor
  Serial.println("Buzzer initialized...");

  // --- First Beep Test (Blocking) ---
  // Play a 175 Hz tone (F3) using a blocking function
  Serial.println("Beep at 175 Hz (F3) and switch off after 1s by calling buzzerOff().");
  buzzerBeepBlocking(175);  // Start tone and block execution
  delay(1000);              // Wait for 1 second
  buzzerOff();              // Turn off the buzzer manually
  delay(1000);              // Wait before next test

  // --- Second Beep Test (Non-Blocking) ---
  // Play a 392 Hz tone (G4) for 1.5 seconds using non-blocking function
  Serial.println("Beep at 392 Hz (G4) and switch off after 1.5s.");
  buzzerBeepNonBlocking(NOTE_G4, 1500);   // Start tone and return immediately
  delay(2000);   // Wait long enough to let the tone finish before next part

  // --- Blocking Melody ---
  // Define a short melody using note frequencies (see kcomp_buzzer.h)
  Serial.println("Play melody (blocking)");
  int melody[] = { NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4 };
  int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4 };  // Duration values (relative to whole note)

  // Play the melody in a blocking way (the function won't return until the song finishes)
  buzzerMelodyBlocking(melody, noteDurations, 8);
  delay(1000); 

  // --- Non-Blocking Melody ---
  // Same melody played again, but now using non-blocking version
  // This requires calling updateMelody() regularly to keep the song playing
  Serial.println("Play melody again (non-blocking). Requires you to call < every 20ms or so.");
  buzzerMelodyNonBlocking(melody, noteDurations, 8);
 
}

// The loop() function runs repeatedly forever after setup()
// Used here to update the non-blocking melody
void loop() {
  updateMelody(); // Check if it's time to play the next note
  delay(20);      // Wait 20ms between checks (good for responsiveness)
}