/*
  Faser.h - Library for handling FSR inputs and converting them to buttons presses.
*/

#include "Arduino.h"
#include "Faser.h"

Faser::Faser(int pinsParam[SENSORS_COUNT], int sensitivitiesParam[SENSORS_COUNT], char keysParam[SENSORS_COUNT + 1], bool debugParam)
{
  for (int i = 0; i < SENSORS_COUNT; i++)
  {
    sensorsPins[i] = pinsParam[i];
    pinMode(sensorsPins[i], INPUT);

    sensorsSensitivities[i] = sensitivitiesParam[i];
    sensorsStates[i] = false;
    lastStateChangeTime[i] = 0;
  }

  debug = debugParam;
  lastKeypressTime = 0;
  strncpy(keys, keysParam, 5);

  processCommandCounter = 0;
}

void Faser::tick()
{
  unsigned long currentTime = micros();

  bool displayDebugTick = false;
  unsigned long lastDebugTimeDiff = (currentTime - lastDebugTime);
  if (lastDebugTimeDiff > DEBUG_INTERVAL)
  {
    lastDebugTime = currentTime;
    displayDebugTick = debug;
  }

  readSerialCommand(displayDebugTick);

  readSensors(currentTime, displayDebugTick);

  // Refresh current time
  currentTime = micros();

  updateKeyPresses(currentTime, displayDebugTick);
}

// Read available data
void Faser::readSerialCommand(bool displayDebugTick)
{
  processCommandCounter = (processCommandCounter + 1);
  if (processCommandCounter == COMMAND_POLL_RATE)
  {
    if (Serial.available() > 0)
    {
      processIncomingData(Serial.read());
    }
    processCommandCounter = 0;
  }
}

// Fill in command buffer before sending off to processing
void Faser::processIncomingData(const byte inByte)
{
  // TODO make that a class member?
  static char inputCommand[MAX_INPUT];
  static unsigned int inputPosition = 0;

  switch (inByte)
  {
  // End of line (command is complete)
  case '\n':
    inputCommand[inputPosition] = 0; // Add null terminator
    processCommand(inputCommand);

    inputPosition = 0;
    break;

  // Discard carriage returns
  case '\r':
    break;

  // Append byte to array unless buffer is already full
  default:
    if (inputPosition < (MAX_INPUT - 1))
    {
      inputCommand[inputPosition++] = inByte;
    }
    break;
  }
}

void Faser::processCommand(char *data)
{
  // Get the command type based on first byte
  // The command is based on the ascii keycode value
  // 48 == 0, 51 == 3...
  int index = data[0] - 48;

  if (index < SENSORS_COUNT)
  {
    data[5] = 0; // Add null byte to limit input value length to 4 numbers
    sensorsSensitivities[index] = atoi((const char *)&(data[1]));
  }
  else
  {
    Serial.print("unrecognized command:");
    Serial.println(index);
  }

  // Print current sensitivities
  char sensitivityStringBuf[5];

  Serial.print("sensors_sensitivity;");
  for (int i = 0; i < SENSORS_COUNT; i++)
  {
    sprintf(sensitivityStringBuf, "%4d", sensorsSensitivities[i]);

    Serial.print("sensor");
    Serial.print(i);
    Serial.print(":");
    Serial.print(&(sensitivityStringBuf[0]));
    Serial.println(";");
  }
  Serial.println("");
}

void Faser::readSensors(unsigned long currentTime, bool displayDebugTick)
{
  for (int i = 0; i < SENSORS_COUNT; i++)
  {
    int sensorValue = analogRead(sensorsPins[i]);

    unsigned long stateChangeTimeDiff = ((unsigned long)(currentTime - lastStateChangeTime[i]));

    if (sensorValue > (sensorsSensitivities[i]))
    {
      // Going from unpressed to pressed and debounce interval has passed
      if (!sensorsStates[i] && (stateChangeTimeDiff >= DEBOUNCE_TIME))
      {
        sensorsStates[i] = true;
        lastStateChangeTime[i] = currentTime;

        dumpSensorValue(i, sensorValue, false, true, stateChangeTimeDiff, debug);
      }
      // Going from unpressed to pressed but debounce interval has not passed
      else if (!sensorsStates[i])
      {
        dumpSensorValue(i, sensorValue, false, false, stateChangeTimeDiff, debug);
      }
      // Sensor was pressed and is still pressed
      else
      {
        dumpSensorValue(i, sensorValue, true, true, stateChangeTimeDiff, displayDebugTick);
      }
    }
    else
    {
      // Going from pressed to unpressed and debounce interval has passed
      if (sensorsStates[i] && (sensorValue < LOWER_LIMIT_PRESSURE) && (stateChangeTimeDiff >= DEBOUNCE_TIME))
      {
        sensorsStates[i] = false;
        lastStateChangeTime[i] = currentTime;

        dumpSensorValue(i, sensorValue, true, false, stateChangeTimeDiff, debug);
      }
      // Going from pressed to unpressed but debounce interval has not passed/pressure is still above lower limit
      else if (sensorsStates[i])
      {
        dumpSensorValue(i, sensorValue, true, true, stateChangeTimeDiff, debug);
      }
    }
  }
}

void Faser::dumpSensorValue(int sensorIdx, int value, bool oldState, bool newState, unsigned long stateChangeTimeDiff, bool displayDebug)
{
  if (!displayDebug)
  {
    return;
  }

  // ensure fixed length of output
  char sensitivityStringBuf[5];
  sprintf(sensitivityStringBuf, "%4d", sensorsSensitivities[sensorIdx]);
  char valueStringBuf[5];
  sprintf(valueStringBuf, "%4d", value);

  char stateChangeStringBuf[33];
  sprintf(stateChangeStringBuf, "%*d", 32, stateChangeTimeDiff);

  Serial.print("sensor_state;");
  Serial.print("sensor:");
  Serial.print(sensorIdx);
  Serial.print(";sensitivity:");
  Serial.print(&(sensitivityStringBuf[0]));
  Serial.print(";value:");
  Serial.print(&(valueStringBuf[0]));
  Serial.print(";previous_state:");
  Serial.print(oldState);
  Serial.print(";new_state:");
  Serial.print(newState);
  Serial.print(";state_change_time_diff:");
  Serial.print(&(stateChangeStringBuf[0]));
  Serial.println("");
}

void Faser::updateKeyPresses(unsigned long currentTime, bool _displayDebugTick)
{
  unsigned long keypressChangeTimeDiffInMs = (unsigned long)(currentTime - lastKeypressTime);

  if (keypressChangeTimeDiffInMs > (double)(DELAY_TIME))
  {
    for (int i = 0; i < SENSORS_COUNT; i++)
    {
      if (sensorsStates[i])
      {
        Keyboard.press(keys[i]);
      }
      else
      {
        Keyboard.release(keys[i]);
      }
    }

    lastKeypressTime = currentTime;
  }
}
