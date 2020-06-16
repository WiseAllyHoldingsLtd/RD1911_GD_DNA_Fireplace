#include "TriacDriver.h"
#include "Constants.h"


TriacDriver::TriacDriver(RUInterface &ru) : m_regUnit(ru)
{
}


bool TriacDriver::turnOn(void)
{
  bool result = m_regUnit.updateConfig(RURegulator::REGULATOR_SETPOINT, false, false,  Constants::MAX_SETP_TEMPERATURE);

  if (result)
  {
    result = m_regUnit.updateExternalTemperatureFP(Constants::MIN_SETP_TEMPERATURE);
  }

  return result;
}


bool TriacDriver::turnOff(void)
{
  return m_regUnit.updateConfig(RURegulator::REGULATOR_OFF, Constants::USE_INTERNAL_TEMP, Constants::ENABLE_RU_POT,  Constants::HEATER_OFF_SET_POINT);
}
