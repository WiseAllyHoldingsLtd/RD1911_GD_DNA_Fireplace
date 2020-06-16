#pragma once

#include "RUInterface.h"


class TriacDriver
{
public:
  TriacDriver(RUInterface &ru);

  /**
   * Send command to turn triac ON by using low external temp and high setpoint.
   * @remark: RU-SW limits triac ON/OFF interval to max. once per 100 secs.
   */
  bool turnOn(void);

  /**
   * Send command to turn triac OFF.
   * @remark: RU-SW limits triac ON/OFF interval to max. once per 100 secs.
   */
  bool turnOff(void);

private:

  RUInterface &m_regUnit;
};
