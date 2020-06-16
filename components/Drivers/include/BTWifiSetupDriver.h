#pragma once

#include "BTWifiSetupDriverInterface.h"
#include "BTDriverInterface.h"
#include "Constants.h"
#include "EventFlags.hpp"
#include "Settings.h"
#include "SoftwareResetDriverInterface.h"

static_assert(sizeof(char) == 1, "char is not 1 byte");


class BTWifiSetupDriver: public BTWifiSetupDriverInterface,
                         public BTDriverEventReceiverInterface
{
public:
  BTWifiSetupDriver(BTDriverInterface &btDriver, uint32_t btPasskey, Settings &settings, SoftwareResetDriverInterface &swResetDriver);

  BTWifiSetupDriver(const BTWifiSetupDriver &) = delete;
  BTWifiSetupDriver &operator=(const BTWifiSetupDriver &) = delete;

  virtual bool startGattService();
  virtual void stopGattService();

  virtual bool isGattServiceRunning();

  virtual bool setConnectionStatus(WifiConnectionStatus::Enum connectionStatus,
                                   const char *connectionStatusString);

  virtual bool waitForReconnectCmdReceived(uint32_t maxTime) const;
  virtual void registerReconnectCmdConsumed(void);

  virtual void getWifiSSID(char * ssid, uint32_t size);
  virtual void getWifiPassword(char * password, uint32_t size);
  virtual uint8_t getWifiConnectionType(void);
  virtual uint32_t getStaticIp() const;
  virtual uint32_t getStaticNetmask() const;
  virtual uint32_t getStaticGateway() const;
  virtual uint32_t getStaticDns1() const;
  virtual uint32_t getStaticDns2() const;

  virtual void setWifiSSID(char * ssid);
  virtual void setWifiPassword(char * password);

  virtual void setStaticIpInUse();
  virtual void setDynamicIpInUse();

private:
  void reset();
  //bool init();

  bool validConnectionStatusString(const char *str) const;

  void handleCharacteristicValueChanged(GattAttributeIndex attributeIndex,
                                        uint16_t size,
                                        uint8_t *value);

  void updateDhcpStaticIp();

  char m_ssid[Constants::WIFI_SSID_SIZE_MAX];
  char m_password[Constants::WIFI_PASSWORD_SIZE_MAX];
  uint8_t m_connectionType; /* TODO: What does this contain? Need a definition. */
  uint32_t m_staticIp;
  uint32_t m_staticNetmask;
  uint32_t m_staticGateway;
  uint32_t m_staticDns1;
  uint32_t m_staticDns2;
  uint32_t m_proxyServer;
  uint16_t m_proxyPort;
  char m_proxyUsername[Constants::PROXY_USERNAME_SIZE_MAX];
  char m_proxyPassword[Constants::PROXY_PASSWORD_SIZE_MAX];
  uint16_t m_connectionStatus;
  char m_connectionStatusString[Constants::CONNECTION_STATUS_STRING_SIZE_MAX];

  bool m_dhcpStaticIp;
  bool m_dhcpManualProxy;

  uint16_t m_connectionStatusCc;
  uint16_t m_connectionStatusStringCc;

  BTDriverInterface &m_btDriver;
  uint32_t m_btPasskey;

  char m_btValEOLT;
  char m_btValTEST;
  char m_btValSWVer[Constants::SW_VER_BT_SIZE_MAX];
  Settings &m_settings;
  SoftwareResetDriverInterface &m_swResetDriver;

  EventFlags m_eventFlags;
  static const uint8_t SSID_RECEIVED_EVENT_ID = 0U;
  static const uint8_t PASSWORD_RECEIVED_EVENT_ID = 1U;
  static const uint8_t CONNECTION_TYPE_RECEIVED_EVENT_ID = 2U;

  static const uint8_t APP_RECONNECT_CMD_RECEIVED_EVENT_ID = 3U;
};
