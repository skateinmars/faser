/*
  Faser.h - Library for handling FSR inputs and converting them to buttons presses.
*/

#ifndef Faser_h
#define Faser_h

#include "Arduino.h"
#include "Keyboard.h"

#define SENSORS_COUNT 4

#define FASER_COMMAND_

const int LOWER_LIMIT_PRESSURE = 1000;        // Minimum value for "unpress" to happen
const int DEFAULT_PRESSURE_SENSITIVITY = 500; // Default sensitivity

// Interval times in micros
const long DELAY_TIME = 2500;         // Delay is the time between which keyboard command doesn't change
const long DEBOUNCE_TIME = 10 * 1000; // Debounce is the time between which sensor changes are ignored
const long DEBUG_INTERVAL = 1000000;

const unsigned int COMMAND_POLL_RATE = 10 - 1; // Process Serial commands every n ticks
const unsigned int MAX_INPUT = 10;             // Max number of bytes to accept in a command

class Faser
{
public:
  Faser(int pinsParam[SENSORS_COUNT], int sensitivitiesParam[SENSORS_COUNT], char keysParam[SENSORS_COUNT + 1], bool debugParam);
  void tick();

private:
  bool debug;                                       // Display debug info on Serial
  char keys[SENSORS_COUNT + 1];                     // Keyboard keys to press
  int sensorsPins[SENSORS_COUNT];                   // Pins connected to FSR/resistors
  int sensorsSensitivities[SENSORS_COUNT];          // Sensitivity settings
  bool sensorsStates[SENSORS_COUNT];                // Store whether sensor is pressed
  unsigned long lastStateChangeTime[SENSORS_COUNT]; // timestamp for the last time a sensor state was updated
  unsigned long lastKeypressTime;                   // timestamp for the last time a key command was sent
  unsigned long lastDebugTime;                      // timestamp for the last time a key command was sent
  int processCommandCounter;                        // Ticks counter for command processing

  void readSensors(unsigned long currentTime, bool displayDebug);
  void updateKeyPresses(unsigned long currentTime, bool displayDebug);
  void dumpSensorValue(int sensorIdx, int value, bool oldState, bool newState, unsigned long stateChangeTimeDiff, bool displayDebug);
  void readSerialCommand(bool displayDebugTick);
  void processIncomingData(const byte inByte);
  void processCommand(char *data);
};

#endif
