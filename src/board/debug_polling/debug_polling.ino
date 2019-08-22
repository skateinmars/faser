// Quick program to test polling rate of a system using evzh
// Wire a button to A0 to spam joystick button commands.

#include <Joystick.h>

int buttonPin = A0;

int buttonState;

bool pressed = false;

Joystick_ Joystick = Joystick_(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                               2, 0,                 // Button Count, Hat Switch Count
                               false, false, false,  // X and Y, but no Z Axis
                               false, false, false,  // No Rx, Ry, or Rz
                               false, false,         // No rudder or throttle
                               false, false, false); // No accelerator, brake, or steering

void setup()
{
  pinMode(buttonPin, INPUT);
  Joystick.begin();
}

void loop()
{
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH)
  {
    if (!pressed)
    {
      Joystick.setButton(0, 1);
      Joystick.setButton(1, 1);

      pressed = false;
    }
    else
    {
      Joystick.setButton(0, 0);
      Joystick.setButton(1, 0);
    }
  }
  else
  {
    Joystick.setButton(0, 0);
    Joystick.setButton(1, 0);
  }
}
