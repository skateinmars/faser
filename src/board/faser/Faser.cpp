/*
  Faser.h - Library for handling FSR inputs and converting them to buttons presses.
*/

#include "Arduino.h"
#include "Faser.h"

Faser::Faser(int pinsParam[SENSORS_COUNT], int sensitivityChangeThresholdParam, int sensitivitiesParam[SENSORS_COUNT], char keysParam[SENSORS_COUNT + 1], bool debugParam)
{
  for (int i = 0; i < SENSORS_COUNT; i++)
  {
    sensorsPins[i] = pinsParam[i];

    sensorsSensitivities[i] = sensitivitiesParam[i];
    sensorsStates[i] = false;
    lastStateChangeTime[i] = 0;
    activeSensorsValue[i].begin(SMOOTHED_EXPONENTIAL, 40);
    initialSensitivities[i] = 0;
    previousSensorsValue[i] = 0;
  }

  debug = debugParam;
  strncpy(keys, keysParam, SENSORS_COUNT);

  debounceTime = INITIAL_DEBOUNCE_TIME;

  processCommandCounter = 0;

  sensitivityChangeThreshold = sensitivityChangeThresholdParam;
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

    initialSensitivities[index] = 0; // Also reset initial sensitivity

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

    // Print updated debounce value
    char debounceStringBuf[6];
    sprintf(debounceStringBuf, "%5ld", debounceTime);

    Serial.print("debounce_time|");
    Serial.print("debounce:");
    Serial.print(&(debounceStringBuf[0]));
    Serial.println(";");
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
    activeSensorsValue[i].add(analogRead(sensorsPins[i]));
    int activeValue = activeSensorsValue[i].get();

    unsigned long stateChangeTimeDiff = ((unsigned long)(currentTime - lastStateChangeTime[i]));

    // Set initial sensitivity value
    if (initialSensitivities[i] == 0)
    {
      initialSensitivities[i] = activeValue - 50; // 50 less so sensor can "go back" to a value close but higher than initial one
      previousSensorsValue[i] = activeValue;
      return;
    }

    if (activeValue > initialSensitivities[i])
    {
      bool isMovingToUp = false;
      bool isMovingToDown = false;

      if (activeValue > (previousSensorsValue[i] + sensitivityChangeThreshold))
      {
        isMovingToUp = true;
        previousSensorsValue[i] = activeValue;
      }
      else if (activeValue < (previousSensorsValue[i] - sensitivityChangeThreshold))
      {
        isMovingToDown = true;
        previousSensorsValue[i] = activeValue;
      }

      if (isMovingToUp)
      {
        // Going from unpressed to pressed
        if (!sensorsStates[i])
        {
          sensorsStates[i] = true;
          lastStateChangeTime[i] = currentTime;
          updateKeyPress(i, true);

          dumpSensorValue(i, activeSensorsValue[i].getLast(), activeValue, false, true, stateChangeTimeDiff, debug);
        }
        // Sensor was pressed and is still pressed
        else
        {
          dumpSensorValue(i, activeSensorsValue[i].getLast(), activeValue, true, true, stateChangeTimeDiff, displayDebugTick);
        }

        previousSensorsValue[i] = activeValue;
      }
      else if (isMovingToDown)
      {
        // Going from pressed to unpressed
        if (sensorsStates[i])
        {
          sensorsStates[i] = false;
          lastStateChangeTime[i] = currentTime;
          updateKeyPress(i, false);

          dumpSensorValue(i, activeSensorsValue[i].getLast(), activeValue, true, false, stateChangeTimeDiff, debug);
        }
        // Is unpressed, so log only if value is close to limit to avoid flooding
        else if (activeSensorsValue[i].getLast() > (sensorsSensitivities[i] - sensorSensitivityDebugThreshold))
        {
          dumpSensorValue(i, activeSensorsValue[i].getLast(), activeValue, false, false, stateChangeTimeDiff, debug);
        }

        previousSensorsValue[i] = activeValue;
      }
      else if (sensorsStates[i] && activeValue > previousSensorsValue[i])
      {
        // Highest value is the new normal
        previousSensorsValue[i] = activeValue;
      }
      else if (!sensorsStates[i] && activeValue < previousSensorsValue[i])
      {
        // Lowest value is the new normal
        previousSensorsValue[i] = activeValue;
      }
    }
    else
    {
      // Going from pressed to unpressed
      if (sensorsStates[i])
      {
        sensorsStates[i] = false;
        lastStateChangeTime[i] = currentTime;
        updateKeyPress(i, false);
        previousSensorsValue[i] = initialSensitivities[i]; // Reset

        dumpSensorValue(i, activeSensorsValue[i].getLast(), activeValue, true, false, stateChangeTimeDiff, debug);
      }
      // Is unpressed, so log only if value is close to limit to avoid flooding
      else if (activeSensorsValue[i].getLast() > (sensorsSensitivities[i] - sensorSensitivityDebugThreshold))
      {
        dumpSensorValue(i, activeSensorsValue[i].getLast(), activeValue, false, false, stateChangeTimeDiff, debug);
      }
    }
  }
}

void Faser::dumpSensorValue(int sensorIdx, int rawValue, int smoothedValue, bool oldState, bool newState, unsigned long stateChangeTimeDiff, bool displayDebug)
{
  if (!displayDebug)
  {
    return;
  }

  // ensure fixed length of output
  char sensitivityStringBuf[5];
  sprintf(sensitivityStringBuf, "%4d", sensorsSensitivities[sensorIdx]);
  char rawValueStringBuf[5];
  sprintf(rawValueStringBuf, "%4d", rawValue);
  char smoothedValueStringBuf[5];
  sprintf(smoothedValueStringBuf, "%4d", smoothedValue);

  Serial.print("sensor_state|");
  Serial.print("sensor:");
  Serial.print(sensorIdx);
  Serial.print(";sensitivity:");
  Serial.print(&(sensitivityStringBuf[0]));
  Serial.print(";raw_value:");
  Serial.print(&(rawValueStringBuf[0]));
  Serial.print(";smoothed_value:");
  Serial.print(&(smoothedValueStringBuf[0]));
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
