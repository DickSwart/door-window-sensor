#ifndef SwartNinjaReedSwitch_H
#define SwartNinjaReedSwitch_H

#include "Arduino.h"

class SwartNinjaReedSwitch
{
public:
  SwartNinjaReedSwitch(int pin, void (*callback)(bool, int), int measureInterval = 500);
  void init(void);
  void loop(void);
  bool getCurrentState(void);

private:
  int _pin;
  int _measureInterval;
  bool _currentState;
  void (*_callback)(bool, int);
  bool _readState(void);
};

#endif