#pragma once

#include <stdint.h>

enum RURegulator {
  REGULATOR_SETPOINT = 0,
  REGULATOR_CU_POT = 1,  // CU Pot Meter
  REGULATOR_RU_POT = 2,  // RU Pot meter
  REGULATOR_OFF = 3,  // Off (No heat)
};

/*
 * This class uses Fixed point decimal numbers with 3 decimal places. A value of 1024 is really 1.024
 */
class RUInterface
{
public:
  /*
   * Check that device is performing as expected
   * Returns true if OK
   */
  virtual bool performSelfTest() = 0;

  /*
   * Update RU config / setpoint (degrees Celsius)
   * Returns true if OK
   */
  virtual bool updateConfig(RURegulator regulator, bool useInternalTemp, bool enableRUPot, int32_t newSetpointFP) = 0;

  /*
   * Returns SW version of RU or -1 if unknown/failure
   */
  virtual int32_t getSWVersion() = 0;

  /*
   * Returns true if this is a HighPower RU (decided by SW version)
   */
  virtual bool isHighPowerVersion() const = 0;

  /*
   * Send updated external temperature reading to RU. In degrees Celsius
   * Returns true if OK
   */
  virtual bool updateExternalTemperatureFP(int32_t temperatureFP) = 0;

  /*
   * Returns RU's estimated room temperature in degrees Celsius
   * or INT32_MIN if failure
   */
  virtual int32_t getEstimatedRoomTemperatureFP() = 0;

  /*
   * Returns value if RU pot meter in degrees Celsius.
   * Could be RU internal, CU setpoint or disabled based on configuration.
   * Returns Int32_MIN on failure or disabled.
   */
  virtual int32_t getRegulatorPotMeterFP() = 0;

  /*
   * Returns true if RU heating element is active.
   * In case of failure, false is returned.
   */
  virtual bool isHeatElementActive() = 0;

  virtual ~RUInterface() {}
};
