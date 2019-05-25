/*
  Faser.h - Library for handling FSR inputs and converting them to buttons presses.
*/

#ifndef Faser_h
#define Faser_h

#include "Arduino.h"
#include "Keyboard.h"
#include "Smoothed.h"

#define SENSORS_COUNT 4

const int DEFAULT_PRESSURE_SENSITIVITY = 500; // Default sensitivity
const int MIN_PRESSURE_SENSITIVITY = 300;     // Minimum sensitivity

const int sensorSensitivityDebugThreshold = 10; // Apply this threshold when logging values

// Interval times in micros
const long INITIAL_DEBOUNCE_TIME = 10 * 1000;
const long DEBUG_INTERVAL = 1000000;

const int previousValueWeight = 1; // Weight to apply to previous reading

const unsigned int COMMAND_POLL_RATE = 10 - 1; // Process Serial commands every n ticks
const unsigned int MAX_INPUT = 10;             // Max number of bytes to accept in a command

class Faser
{
public:
  Faser(int pinsParam[SENSORS_COUNT], int sensitivitiesParam[SENSORS_COUNT], char keysParam[SENSORS_COUNT + 1], bool debugParam);
  void tick();

private:
  bool debug;                                        // Display debug info on Serial
  char keys[SENSORS_COUNT + 1];                      // Keyboard keys to press
  int sensorsPins[SENSORS_COUNT];                    // Pins connected to FSR/resistors
  int sensorsSensitivities[SENSORS_COUNT];           // Sensitivity settings
  Smoothed<int> previousSensorsValue[SENSORS_COUNT]; // Previous reading of sensors
  bool sensorsStates[SENSORS_COUNT];                 // Store whether sensor is pressed
  unsigned long lastStateChangeTime[SENSORS_COUNT];  // timestamp for the last time a sensor state was updated
  long debounceTime;                                 // Debounce is the time between which sensor changes are ignored
  unsigned long lastDebugTime;                       // timestamp for the last time a key command was sent
  int processCommandCounter;                         // Ticks counter for command processing

  void readSensors(unsigned long currentTime, bool displayDebug);
  void updateKeyPress(int sensorIdx, bool isPressed);
  void dumpSensorValue(int sensorIdx, int rawValue, int smoothedValue, bool oldState, bool newState, unsigned long stateChangeTimeDiff, bool displayDebug);
  void readSerialCommand(bool displayDebugTick);
  void printSensitivity(int index, int sensorsSensitivity);
  void processIncomingData(const byte inByte);
  void processCommand(char *data);
};

#endif
