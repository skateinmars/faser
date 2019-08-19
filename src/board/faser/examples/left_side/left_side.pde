#include <Faser.h>

bool DEBUG = true;

int PINS[4] = {A0, A1, A2, A3};              // Pins connected to FSR/resistors
int SENSITIVITIES[4] = {800, 800, 800, 800}; // Sensitivity settings

Faser faser(PINS, SENSITIVITIES, DEBUG);

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  faser.tick();
}
