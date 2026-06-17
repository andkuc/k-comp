#include "kcomp_buzzer.h"

_buzzerState buzzerState;

unsigned int* _notes = 0;
unsigned int* _notesDurations = 0;
byte _notesLength = 0;
byte _notesCurrentIndex = 0;

unsigned long _notesStartTime = 0; 
unsigned long _notesPauseTime = 0;

void initBuzzer() {
     buzzerState = BUZZER_INIT;
  
	   pinMode(BUZZER_PIN, OUTPUT);
}

void buzzerBeepBlocking(unsigned int frequency) {
    tone(BUZZER_PIN, frequency);
}
void buzzerBeepNonBlocking(unsigned int frequency, unsigned long duration) {
    tone(BUZZER_PIN, frequency, duration);
}

/*void buzzerBeepNonBlocking(unsigned int frequency) {
    tone(BUZZER_PIN, frequency);
}
void buzzerBeepNonBlocking(unsigned int frequency, unsigned long duration) {
    tone(BUZZER_PIN, frequency, duration);
}
*/
void buzzerOff() {
    buzzerState = BUZZER_INIT;
     noTone(BUZZER_PIN);
}

void buzzerMelodyBlocking(unsigned int notes[], unsigned int durations[], byte length) {
    for(byte i=0; i < length; i++) {
        unsigned long duration = 1000 / durations[i];
        tone(BUZZER_PIN, notes[i], duration);
        delay((unsigned long)(duration*BUZZER_PAUSE_MELODY));
        noTone(BUZZER_PIN);
    }
}

void buzzerMelodyNonBlocking(unsigned int notes[], unsigned int durations[], byte length) {
    _notes = notes;
    _notesDurations = durations;
    _notesLength = length;
    _notesCurrentIndex = 0;
    _notesPauseTime = 0;

    _notesStartTime = millis();

    buzzerState = BUZZER_MELODY;
}

void updateMelody() {
    switch(buzzerState) {
        case BUZZER_INIT: 
            break;
        case BUZZER_MELODY:
            if(_notesCurrentIndex < _notesLength) {
                if(!_notesPauseTime) {
                    unsigned long duration = 1000 / _notesDurations[_notesCurrentIndex];
                    tone(BUZZER_PIN, _notes[_notesCurrentIndex], duration);
                    _notesPauseTime = (unsigned long)(duration*BUZZER_PAUSE_MELODY);
                    _notesStartTime = millis();
                }
                if(millis() - _notesStartTime > _notesPauseTime) {
                    noTone(BUZZER_PIN);
                    _notesPauseTime = 0;
                    _notesCurrentIndex++;
                }
            } else {
              noTone(BUZZER_PIN);
              buzzerState = BUZZER_INIT;
            }
    }
}

void updateBuzzer() {

}
