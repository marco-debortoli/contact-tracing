#include "led.h"


LEDModule::LEDModule(int pin) {
    _ledPin = pin;
    pinMode(_ledPin, OUTPUT);
    digitalWrite(_ledPin, LOW);
}

void LEDModule::quickBlink() {
    digitalWrite(_ledPin, HIGH);
    delay(250);
    digitalWrite(_ledPin, LOW);
    delay(250);
}

void LEDModule::mediumBlink() {
    digitalWrite(_ledPin, HIGH);
    delay(500);
    digitalWrite(_ledPin, LOW);
    delay(500);
}

void LEDModule::longBlink() {
    digitalWrite(_ledPin, HIGH);
    delay(1000);
    digitalWrite(_ledPin, LOW);
    delay(1000);
}

void LEDModule::on() {
    digitalWrite(_ledPin, HIGH);
}

void LEDModule::off() {
    digitalWrite(_ledPin, LOW);
}