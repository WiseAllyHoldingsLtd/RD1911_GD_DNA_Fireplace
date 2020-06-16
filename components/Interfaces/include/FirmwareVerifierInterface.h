#pragma once
#include <cstdint>

class FirmwareVerifierInterface
{
public:
  virtual void saveToSettings(void) = 0;
  virtual const char * getFirmwareUpgradeID(void) const = 0;
  virtual uint32_t getFirmwareVersionCode(void) const = 0;
  virtual uint8_t getFirmwareUpgradeCount(void) const = 0;
  virtual bool isUpgrading(void) const = 0;
  virtual bool isUpgradeBlocked(void) const = 0;
  virtual bool registerUpgradeAttempt(const char * upgradeUrl) = 0;
  virtual bool verifyUpgrade(void) = 0;

  virtual ~FirmwareVerifierInterface(void) {}
};
