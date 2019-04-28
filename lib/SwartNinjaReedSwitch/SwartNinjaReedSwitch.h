#ifndef SwartNinjaReedSwitch_H
#define SwartNinjaReedSwitch_H

#include "Arduino.h"

class SwartNinjaReedSwitch
{
public:
  SwartNinjaReedSwitch(int pin, void (*callback)(bool, int), int measureInterval = 100);
  void init(void);
  void loop(void);
  bool getCurrentState(void);
  int getPinNumber(void);

private:
  int _pin;
  bool _currentState;
  int _measureInterval;
  void (*_callback)(bool, int);
  bool _readState(void);
};

#endif