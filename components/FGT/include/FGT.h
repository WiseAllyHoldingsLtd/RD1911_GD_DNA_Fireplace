#pragma once

#include <cstdint>

#include "DeviceMeta.h"
#include "BTDriverInterface.h"
#include "Settings.h"
#include "SoftwareResetDriverInterface.h"
#include "CryptoDriverInterface.h"
#include "DisplayDriverInterface.h"
#include "QTouchDriver.h"
#include "TriacDriver.h"
#include "OTADriverInterface.h"
#include "FGTWifiStatus.h"
#include "WifiDriverInterface.h"

namespace
{
  #define FGT_OTA_EN   0
}

class FGT: public BTDriverEventReceiverInterface
{
public:
  FGT(DeviceMeta &meta,
      BTDriverInterface &btDriver,
      Settings &settings,
      SoftwareResetDriverInterface &swResetDriver,
      CryptoDriverInterface &cryptoDriver,
      DisplayDriverInterface &displayDriver,
      TouchDriverInterface &touchDriver,
      RUInterface &regUnit,
      OTADriverInterface &otaDriver,
      WifiDriverInterface &wifiDriver);

  virtual bool startGattService();
  virtual void stopGattService();

  void runTest(void);

private:
  DeviceMeta & m_meta;
  BTDriverInterface &m_btDriver;
  Settings &m_settings;
  SoftwareResetDriverInterface &m_swResetDriver;
  CryptoDriverInterface &m_cryptoDriver;
  DisplayDriverInterface &m_displayDriver;
  TouchDriverInterface &m_touchDriver;
  RUInterface &m_regUnit;
  TriacDriver * m_triacDriver;
  OTADriverInterface &m_otaDriver;
  WifiDriverInterface &m_wifiDriver;

  // BLE values
  char m_btValLCD[Constants::LCD_BT_SIZE_MAX];
  char m_btValTRIAC[Constants::TRIAC_BT_SIZE_MAX];
  char m_btValNTC[Constants::NTC_BT_SIZE_MAX];
  char m_btValEOLT;
  char m_btValTEST;
  char m_btValPASSKEY[Constants::PASSKEY_BT_SIZE_MAX];
  char m_btValGDID[Constants::GDID_BT_SIZE_MAX];
  char m_btValFULL_INFO[Constants::FULL_INFO_BT_SIZE_MAX];
  char m_btValSWVer[Constants::SW_VER_BT_SIZE_MAX];
  FgtWifiStatus::Enum m_btValWifiStatus;
#if FGT_OTA_EN
  char m_btValOTAEn;
  char m_btValCMD[Constants::OTA_CMD_BT_SIZE_MAX];
#endif
  uint16_t m_btValCMDCcc;

  uint8_t m_cryptoSerial[9];
  uint8_t m_displayText[Constants::TEXT_MAX_LENGTH];
  int32_t m_temperature;
#if FGT_OTA_EN
  uint8_t m_OTA_method;
  uint32_t m_OTA_row;
  uint8_t m_OTA_reset_flag;
#endif
  void handleCharacteristicValueChanged(GattAttributeIndex attributeIndex,
                                        uint16_t size,
                                        uint8_t *value);
  void updateDatabase(void);
  void performWifiTest(void);
};

