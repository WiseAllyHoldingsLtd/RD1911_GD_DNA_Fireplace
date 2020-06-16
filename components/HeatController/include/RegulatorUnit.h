#pragma once

#include "RUInterface.h"
#include "TWIDriverInterface.h"
#include <stdint.h>


class RegulatorUnit : public RUInterface
{
public:
  static const uint8_t RU_SLAVE_ADDRESS = 1u;

  RegulatorUnit(TWIDriverInterface &twiDriver);

  virtual bool performSelfTest();
  virtual bool updateConfig(RURegulator regulator, bool useInternalTemp, bool enableRUPot, int32_t newSetpointFP);
  virtual int32_t getSWVersion();
  virtual bool isHighPowerVersion() const;
  virtual bool updateExternalTemperatureFP(int32_t temperatureFP);
  virtual int32_t getEstimatedRoomTemperatureFP();
  virtual int32_t getRegulatorPotMeterFP();
  virtual bool isHeatElementActive();

private:
  /*
   * Calculates the Fletcher-16 checksum of data array with length numBytes.
   */
  uint16_t calculateChecksum(const uint8_t data[], uint32_t numBytes) const;

  /*
   * Calculates check bytes from checksum.
   * Check bytes are appended to the datastream to give the total stream a checksum of 0.
   * The two check bytes are written to buffer.
   */
  void getCheckBytesFromChecksum(uint16_t checksum, uint8_t buffer[2]) const;

  TWIDriverInterface &m_twiDriver;
  int32_t m_swVersion;
};
