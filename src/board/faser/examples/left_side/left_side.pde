#include <Faser.h>

bool DEBUG = true;

uint8_t REPORT_ID = 0x04;

int PINS[4] = {A0, A1, A2, A3};              // Pins connected to FSR/resistors
int SENSITIVITIES[4] = {800, 800, 800, 800}; // Sensitivity settings

Faser faser(PINS, SENSITIVITIES, REPORT_ID, DEBUG);

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  faser.tick();
}
