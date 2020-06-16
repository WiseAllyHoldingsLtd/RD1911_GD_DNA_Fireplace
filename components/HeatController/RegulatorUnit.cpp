#include "RegulatorUnit.h"

#define RU_HIGH_POWER_SW_VERSION_MIN 16


RegulatorUnit::RegulatorUnit(TWIDriverInterface &twiDriver) : m_twiDriver(twiDriver), m_swVersion(-1)
{
}

bool RegulatorUnit::performSelfTest()
{
  uint8_t buffer[1];
  buffer[0] = 5u; // CMD: poll heat element on/off

  // Just check for successful communication. We don't care about the answer :)
  return m_twiDriver.sendRecieve(RU_SLAVE_ADDRESS, buffer, 1u, 1u);
}

bool RegulatorUnit::updateConfig(RURegulator regulator, bool useInternalTemp, bool enableRUPot, int32_t newSetpointFP)
{
  uint8_t buffer[7];
  bool result = false;
  buffer[0] = 1u; // CMD: set new setpoint/config
  buffer[1] = static_cast<uint8_t>(regulator);
  if (!useInternalTemp) {
    buffer[1] |= (1u << 2);
  }

  if (!enableRUPot) {
    buffer[1] |= (1u << 3);
  }

  buffer[2] = static_cast<uint8_t>(static_cast<uint32_t>(newSetpointFP) & 0xffu);
  buffer[3] = static_cast<uint8_t>((static_cast<uint32_t>(newSetpointFP) >> 8) & 0xffu);
  buffer[4] = static_cast<uint8_t>((static_cast<uint32_t>(newSetpointFP) >> 16) & 0xffu);

  uint16_t checkSum = calculateChecksum(buffer, 5u);
  getCheckBytesFromChecksum(checkSum, &buffer[5]);  // writes check bytes straight into buffer

  if (m_twiDriver.sendRecieve(RU_SLAVE_ADDRESS, buffer, 7u, 1u)) {
    // Should always reply with bit 1-3 set
    if ((buffer[0] & 7u) == 7u) {
      // bit 4-8 contains RU SW version.
      m_swVersion = buffer[0] >> 3;
      result = true;
    }
  }

  return result;
}

int32_t RegulatorUnit::getSWVersion()
{
  return m_swVersion;
}

bool RegulatorUnit::isHighPowerVersion() const
{
  return (m_swVersion >= RU_HIGH_POWER_SW_VERSION_MIN);
}

bool RegulatorUnit::updateExternalTemperatureFP(int32_t temperatureFP)
{
  uint8_t buffer[7];
  bool result = false;
  buffer[0] = 2u; // CMD: report externally measured current temp.

  buffer[1] = static_cast<uint8_t>(static_cast<uint32_t>(temperatureFP) & 0xffu);
  buffer[2] = static_cast<uint8_t>((static_cast<uint32_t>(temperatureFP) >> 8) & 0xffu);
  buffer[3] = static_cast<uint8_t>((static_cast<uint32_t>(temperatureFP) >> 16) & 0xffu);
  buffer[4] = static_cast<uint8_t>((static_cast<uint32_t>(temperatureFP) >> 24) & 0xffu);

  uint16_t checkSum = calculateChecksum(buffer, 5u);
  getCheckBytesFromChecksum(checkSum, &buffer[5]);  // writes check bytes straight into buffer

  if (m_twiDriver.sendRecieve(RU_SLAVE_ADDRESS, buffer, 7u, 1u)) {
    // Should always return 3
    result = (buffer[0] == 3u);
  }

  return result;
}

int32_t RegulatorUnit::getEstimatedRoomTemperatureFP()
{
  uint8_t buffer[6];
  int32_t result = INT32_MIN;
  buffer[0] = 3u; // CMD: Get current estimated room temp.

  if (m_twiDriver.sendRecieve(RU_SLAVE_ADDRESS, buffer, 1u, 6u)) {
    uint16_t checksum = calculateChecksum(buffer, 6u);
    if (checksum == 0u) {
      result = static_cast<int32_t>(buffer[0]
            | (static_cast<uint32_t>(buffer[1]) << 8)
            | (static_cast<uint32_t>(buffer[2]) << 16)
            | (static_cast<uint32_t>(buffer[3]) << 24));
    }
  }

  return result;
}

int32_t RegulatorUnit::getRegulatorPotMeterFP()
{
  uint8_t buffer[6];
  int32_t result = INT32_MIN;
  buffer[0] = 4u; // CMD: get RU/CU potmeter value

  if (m_twiDriver.sendRecieve(RU_SLAVE_ADDRESS, buffer, 1u, 6u)) {
    uint16_t checksum = calculateChecksum(buffer, 6u);
    if (checksum == 0u) {
      result = static_cast<int32_t>(buffer[0]
            | (static_cast<uint32_t>(buffer[1]) << 8)
            | (static_cast<uint32_t>(buffer[2]) << 16)
            | (static_cast<uint32_t>(buffer[3]) << 24));
    }
  }
  return result;
}

bool RegulatorUnit::isHeatElementActive()
{
  uint8_t buffer[1];
  bool result = false;
  buffer[0] = 5u; // CMD: poll heat element on/off

  if (m_twiDriver.sendRecieve(RU_SLAVE_ADDRESS, buffer, 1u, 1u)) {
    /* 0: off, 1: on */
    result = (buffer[0] == 0x01u);
  }
  return result;
}

uint16_t RegulatorUnit::calculateChecksum(const uint8_t data[], uint32_t numBytes) const
{
  // Using straigtforward implementation from http://en.wikipedia.org/wiki/Fletcher's_checksum
  uint16_t sum1 = 0u;
  uint16_t sum2 = 0u;

  for (unsigned int index(0u); index < numBytes; ++index)
  {
     sum1 = (sum1 + data[index]) % 255u;
     sum2 = (sum2 + sum1) % 255u;
  }

  return static_cast<uint16_t>(sum2 << 8) | sum1;
}

void RegulatorUnit::getCheckBytesFromChecksum(uint16_t checksum, uint8_t buffer[2]) const
{
  uint8_t f0;
  uint8_t f1;

  f0 = static_cast<uint8_t>(checksum & 0xffu);
  f1 = static_cast<uint8_t>((checksum >> 8) & 0xffu);
  buffer[0] = 0xffu - (( f0 + f1) % 0xffu);
  buffer[1] = 0xffu - (( f0 + buffer[0] ) % 0xffu);
}
