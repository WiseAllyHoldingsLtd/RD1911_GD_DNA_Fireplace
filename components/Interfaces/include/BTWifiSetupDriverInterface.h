#pragma once

#include "WifiConnectionStatus.h"

#include <cstdint>

/*
class BTWifiSetupDriverEventReceiverInterface
{
public:
  ~BTWifiSetupDriverEventReceiverInterface() { }

  virtual void handleAllParamatersSet(
      const char *ssid,
      const char *password,
      ...) = 0;
};
*/

class BTWifiSetupDriverInterface
{
public:
  virtual ~BTWifiSetupDriverInterface() { }

  virtual bool startGattService() = 0;
  virtual void stopGattService() = 0;

  virtual bool isGattServiceRunning() = 0;

  /**
   * @param connectionStatusString
   * Max 32 bytes including NULL-terminator.
   */
  virtual bool setConnectionStatus(WifiConnectionStatus::Enum connectionStatus,
                                   const char *connectionStatusString) = 0;

  /* Wait for an amount of time for WifiSettings to be received from BT. */
  virtual bool waitForReconnectCmdReceived(uint32_t maxTime) const = 0;

  /* Clears the "ReconnectCmdReceived" flag, so that a new "wait" can be started. */
  virtual void registerReconnectCmdConsumed(void) = 0;

  /* Read WifiSettings. Values only valid if waitForWifiSettingsReceived returns true */
  virtual void getWifiSSID(char * ssid, uint32_t size) = 0;
  virtual void getWifiPassword(char * password, uint32_t size) = 0;
  virtual uint8_t getWifiConnectionType(void) = 0;

  virtual uint32_t getStaticIp() const = 0;
  virtual uint32_t getStaticNetmask() const = 0;
  virtual uint32_t getStaticGateway() const = 0;
  virtual uint32_t getStaticDns1() const = 0;
  virtual uint32_t getStaticDns2() const = 0;

  /* Set wifi settings */
  virtual void setWifiSSID(char * ssid) = 0;
  virtual void setWifiPassword(char * password) = 0;

  /* Control the read only boolean dhcpStaticIp characteristic */
  virtual void setStaticIpInUse() = 0;
  virtual void setDynamicIpInUse() = 0;
};
