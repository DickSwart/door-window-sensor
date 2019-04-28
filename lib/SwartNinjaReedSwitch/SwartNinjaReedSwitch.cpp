//include the declaration for this class
#include "SwartNinjaReedSwitch.h"

// Constructor: setup the pin number
SwartNinjaReedSwitch::SwartNinjaReedSwitch(int pin, void (*callback)(bool, int), int measureInterval)
{
  this->_pin = pin;
  this->_callback = callback;
  this->_measureInterval = measureInterval;
}

///////////////////////////////////////////////////////////////////////////
//  Public methods
///////////////////////////////////////////////////////////////////////////

//Setup the reed switch
void SwartNinjaReedSwitch::init(void)
{
  pinMode(_pin, INPUT);
  this->_currentState = this->_readState();
}

//Setup the reed switch
void SwartNinjaReedSwitch::loop(void)
{

  static unsigned long lastSensorCheck = 0;
  if (lastSensorCheck + this->_measureInterval <= millis())
  {
    lastSensorCheck = millis();

    bool newStatus = this->_readState();
    // Check if the value changed since last time
    // we checked
    if (newStatus != this->_currentState)
    {
      this->_currentState = newStatus;

#ifdef DEBUG
      Serial.print("[SwartNinaReedSwitch]: Pin ");
      Serial.print(this->_pin);
      Serial.print(" = ");
      Serial.println(this->_currentState ? "ON" : "OFF");
#endif

      this->_callback(this->_currentState, this->_pin);
    }
  }
}

bool SwartNinjaReedSwitch::getCurrentState(void)
{
  return this->_currentState;
}

int SwartNinjaReedSwitch::getPinNumber(void)
{
  return this->_pin;
}

///////////////////////////////////////////////////////////////////////////
//  Private methods
///////////////////////////////////////////////////////////////////////////

bool SwartNinjaReedSwitch::_readState(void)
{
  return digitalRead(_pin);
}