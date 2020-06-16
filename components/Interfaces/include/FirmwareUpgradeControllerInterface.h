#pragma once
#include <cstdint>
#include "Constants.h"


class FirmwareUpgradeControllerInterface
{
public:
  virtual ~FirmwareUpgradeControllerInterface(void){}

  virtual void setFirmwareUrl(const char * url) = 0;
  virtual bool signalFirmwareUpgradeRequest(void) = 0;
  virtual bool isFirmwareUpgradeRequested(void) const = 0;
  virtual bool initOTA(void) = 0;
  virtual bool commitOTA(void) = 0;
  virtual uint32_t getOTADataSize(void) const = 0;
  virtual bool downloadAndWriteOTAData(uint32_t startByteIndex, uint32_t endByteIndex, uint8_t numOfDownloadAttempts) = 0;
};
