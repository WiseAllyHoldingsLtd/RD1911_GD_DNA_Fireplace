#pragma once

#include "Constants.h"
#include "QueueInterface.hpp"
#include "OTADriverInterface.h"
#include "CloudDriverInterface.h"
#include "FirmwareVerifierInterface.h"
#include "ConnectionStatusChangeRequest.h"
#include "FirmwareUpgradeControllerInterface.h"


class FirmwareUpgradeController : public FirmwareUpgradeControllerInterface
{
public:
  FirmwareUpgradeController(OTADriverInterface &otaDriver, CloudDriverInterface &cloudDriver, FirmwareVerifierInterface &verifier,
                            QueueInterface<ConnectionStatusChangeRequest::Enum> &connStatusChangeQueue);

  virtual void setFirmwareUrl(const char * url);
  virtual bool signalFirmwareUpgradeRequest(void);
  virtual bool isFirmwareUpgradeRequested(void) const;

  virtual bool initOTA(void);
  virtual bool commitOTA(void);
  virtual uint32_t getOTADataSize(void) const;
  virtual bool downloadAndWriteOTAData(uint32_t startByteIndex, uint32_t endByteIndex, uint8_t numOfDownloadAttempts);

private:

  void prepareHttpHeaderRangeValue(uint32_t startByteIndex, uint32_t endByteIndex, char * value) const;

  char m_upgradeUrl[Constants::FW_DOWNLOAD_URL_MAX_LENGTH];
  OTADriverInterface &m_otaDriver;
  CloudDriverInterface &m_cloudDriver;
  FirmwareVerifierInterface & m_verifier;
  QueueInterface<ConnectionStatusChangeRequest::Enum> &m_fwUpdateReadyQueue;
};
