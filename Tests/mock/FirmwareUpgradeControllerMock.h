#pragma once

#include "CppUTestExt\MockSupport.h"

#include "FirmwareUpgradeControllerInterface.h"

#define NAME(method) "FirmwareUpgradeControllerMock::" method


class FirmwareUpgradeControllerMock : public FirmwareUpgradeControllerInterface
{
  virtual void setFirmwareUrl(const char * url)
  {
    mock().actualCall(NAME("setFirmwareUrl")).withStringParameter("url", url);
  }

  virtual bool signalFirmwareUpgradeRequest(void)
  {
    return mock().actualCall(NAME("signalFirmwareUpgradeRequest")).returnBoolValueOrDefault(true);
  }

  virtual bool isFirmwareUpgradeRequested(void) const
  {
    return mock().actualCall(NAME("isFirmwareUpgradeRequested")).returnBoolValueOrDefault(true);
  }

  virtual bool initOTA(void)
  {
    return mock().actualCall(NAME("initOTA")).returnBoolValueOrDefault(true);
  }

  virtual bool commitOTA(void)
  {
    return mock().actualCall(NAME("commitOTA")).returnBoolValueOrDefault(true);
  }

  virtual uint32_t getOTADataSize(void) const
  {
    return mock().actualCall(NAME("getOTADataSize")).returnUnsignedIntValue();
  }

  virtual bool downloadAndWriteOTAData(uint32_t startByteIndex, uint32_t endByteIndex, uint8_t numOfDownloadAttempts)
  {
    return mock().actualCall(NAME("downloadAndWriteOTAData"))
                  .withUnsignedIntParameter("startByteIndex", startByteIndex)
                  .withUnsignedIntParameter("endByteIndex", endByteIndex)
                  .withUnsignedIntParameter("numOfDownloadAttempts", numOfDownloadAttempts)
                  .returnBoolValueOrDefault(true);
  }
};
