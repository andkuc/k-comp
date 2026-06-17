#include "kcomp_led.h"

void initLed() {
    pinMode(LED_PIN, OUTPUT);
}

void ledOn(byte intensity) {
    analogWrite(LED_PIN, map(intensity, 0, 100, 0, 255));
}

void ledOff() {
    digitalWrite(LED_PIN, LOW);
}

void ledFadeIn(byte finalIntensity, int duration) {
    long startTime = millis();
    byte scaledIntensity = map(finalIntensity, 0, 100, 0, 255);

    while(millis() < startTime + duration) {
        byte dutyCycle = scaledIntensity*(1-cos(HALF_PI/duration*(millis() - startTime)));
        analogWrite(LED_PIN, dutyCycle);
    }
}

void ledFadeOut(byte startingIntensity, int duration) {
    long startTime = millis();
    byte scaledIntensity = map(startingIntensity, 0, 100, 0, 255);

    while(millis() < startTime + duration) {
        byte dutyCycle = scaledIntensity*(1-sin(HALF_PI/duration*(millis() - startTime)));
        analogWrite(LED_PIN, dutyCycle);
        delay(50);
    }
}