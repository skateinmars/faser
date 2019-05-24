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
  strncpy(keys, keysParam, 5);

  debounceTime = INITIAL_DEBOUNCE_TIME;

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

    if (sensorsSensitivities[index] < MIN_PRESSURE_SENSITIVITY)
    {
      sensorsSensitivities[index] = MIN_PRESSURE_SENSITIVITY;
    }

    printSensitivity(index, sensorsSensitivities[index]);
  }
  // 'R'ead sensitivities
  else if (index == 34)
  {
    for (int i = 0; i < SENSORS_COUNT; i++)
    {
      printSensitivity(i, sensorsSensitivities[i]);
    }
  }
  // 'D'ebounce time value update
  else if (index == 20)
  {
    data[6] = 0; // Add null byte to limit input value length to 5 numbers
    debounceTime = atol((const char *)&(data[1]));
    if (debounceTime < 0)
    {
      debounceTime = INITIAL_DEBOUNCE_TIME;
    }
  }
  else
  {
    Serial.print("unrecognized_command|code:");
    Serial.print(index);
    Serial.println(";");
  }
}

void Faser::printSensitivity(int index, int sensorsSensitivity)
{
  // Print current sensitivity
  char sensitivityStringBuf[5];
  sprintf(sensitivityStringBuf, "%4d", sensorsSensitivity);

  Serial.print("sensor_sensitivity|");
  Serial.print("sensor:");
  Serial.print(index);
  Serial.print(";sensitivity:");
  Serial.print(&(sensitivityStringBuf[0]));
  Serial.println(";");
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
      if (!sensorsStates[i] && (stateChangeTimeDiff >= debounceTime))
      {
        sensorsStates[i] = true;
        lastStateChangeTime[i] = currentTime;
        updateKeyPress(i, true);

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
      if (sensorsStates[i] && (sensorValue < LOWER_LIMIT_PRESSURE) && (stateChangeTimeDiff >= debounceTime))
      {
        sensorsStates[i] = false;
        lastStateChangeTime[i] = currentTime;
        updateKeyPress(i, false);

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

  // char stateChangeStringBuf[33];
  // sprintf(stateChangeStringBuf, "%*d", 32, stateChangeTimeDiff);

  Serial.print("sensor_state|");
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
  Serial.print(stateChangeTimeDiff);
  Serial.println("");
}

void Faser::updateKeyPress(int sensorIdx, bool isPressed)
{
  if (isPressed)
  {
    Keyboard.press(keys[sensorIdx]);
  }
  else
  {
    Keyboard.release(keys[sensorIdx]);
  }
}
