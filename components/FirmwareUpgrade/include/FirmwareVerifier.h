#pragma once
#include "SettingsInterface.h"
#include "FirmwareVerifierInterface.h"


class FirmwareVerifier : public FirmwareVerifierInterface
{
public:
  FirmwareVerifier(SettingsInterface & settings);
  void init(void);

  virtual void saveToSettings(void);
  virtual const char * getFirmwareUpgradeID(void) const;
  virtual uint32_t getFirmwareVersionCode(void) const;
  virtual uint8_t getFirmwareUpgradeCount(void) const;
  virtual bool isUpgrading(void) const;
  virtual bool isUpgradeBlocked(void) const;

  virtual bool registerUpgradeAttempt(const char * upgradeUrl);
  virtual bool verifyUpgrade(void);

private:
  bool getFirmwareIDFromUrl(const char * url, char * id, uint32_t idSize) const;
  uint32_t calculateVersionCode(void) const;

  SettingsInterface & m_settings;
  char m_firmwareID[Constants::FW_DOWNLOAD_ID_MAX_LENGTH];
  uint32_t m_versionCode;
  uint8_t m_attemptNo;
};
