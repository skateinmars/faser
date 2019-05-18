#include <Arduino.h>
#include <Keyboard.h>
#include <Faser.h>

bool DEBUG = true;

int PINS[4] = {A0, A1, A2, A3};              // Pins connected to FSR/resistors
int SENSITIVITIES[4] = {700, 550, 875, 900}; // Sensitivity settings
char LDUR_KEYS[5] = "cvbn";                  // Keyboard keys to press based on sensor state.

Faser faser(PINS, SENSITIVITIES, LDUR_KEYS, DEBUG);

void setup()
{
  Serial.begin(9600);
  Keyboard.begin();
}

void loop()
{
  faser.tick();
}
