
#pragma once

#include "CppUTestExt\MockSupport.h"
#include "BTWifiSetupDriverInterface.h"

#define NAME(methodName) "BTWifiSetupDriverMock::" methodName

class BTWifiSetupDriverMock : public BTWifiSetupDriverInterface
{
public:
  virtual bool startGattService()
  {
    return mock().actualCall(NAME("startGattService")).returnBoolValueOrDefault(true);
  }

  virtual void stopGattService()
  {
    mock().actualCall(NAME("stopGattService"));
  }

  virtual bool isGattServiceRunning()
  {
    return mock().actualCall(NAME("isGattServiceRunning")).returnBoolValueOrDefault(false);
  }

  virtual bool setConnectionStatus(WifiConnectionStatus::Enum connectionStatus,
                                   const char *connectionStatusString)
  {
    return mock().actualCall(NAME("setConnectionStatus"))
        .withIntParameter("connectionStatus", static_cast<int32_t>(connectionStatus))
        .withStringParameter("connectionStatusString", connectionStatusString)
        .returnBoolValueOrDefault(true);
  }

  virtual bool waitForReconnectCmdReceived(uint32_t maxTime) const
  {
    return mock().actualCall(NAME("waitForReconnectCmdReceived"))
        .withUnsignedIntParameter("maxTime", maxTime)
        .returnBoolValueOrDefault(true);
  }

  virtual void registerReconnectCmdConsumed(void)
  {
    mock().actualCall(NAME("registerReconnectCmdConsumed"));
  }

  virtual void getWifiSSID(char * ssid, uint32_t size)
  {
    mock().actualCall(NAME("getWifiSSID"))
        .withOutputParameter("ssid", ssid)
        .withUnsignedIntParameter("size",  size);
  }

  virtual void getWifiPassword(char * password, uint32_t size)
  {
    mock().actualCall(NAME("getWifiPassword"))
        .withOutputParameter("password", password)
        .withUnsignedIntParameter("size", size);
  }

  virtual uint8_t getWifiConnectionType(void)
  {
    return static_cast<uint8_t>(mock().actualCall(NAME("getWifiConnectionType"))
        .returnUnsignedIntValue());
  }

  virtual void setWifiSSID(char * ssid)
  {
    mock().actualCall(NAME("setWifiSSID")).withStringParameter("ssid", ssid);
  }

  virtual void setWifiPassword(char * password)
  {
    mock().actualCall(NAME("setWifiPassword")).withStringParameter("password", password);
  }

  virtual void setStaticIpInUse()
  {
    mock().actualCall(NAME("setStaticIpInUse"));
  }

  virtual void setDynamicIpInUse()
  {
    mock().actualCall(NAME("setDynamicIpInUse"));
  }

  virtual uint32_t getStaticIp() const
  {
    return mock().actualCall(NAME("getStaticIp"))
        .returnUnsignedIntValueOrDefault(0u);
  }

  virtual uint32_t getStaticNetmask() const
  {
    return mock().actualCall(NAME("getStaticNetmask"))
        .returnUnsignedIntValueOrDefault(0u);
  }

  virtual uint32_t getStaticGateway() const
  {
    return mock().actualCall(NAME("getStaticGateway"))
        .returnUnsignedIntValueOrDefault(0u);
  }

  virtual uint32_t getStaticDns1() const
  {
    return mock().actualCall(NAME("getStaticDns1"))
        .returnUnsignedIntValueOrDefault(0u);
  }

  virtual uint32_t getStaticDns2() const
  {
    return mock().actualCall(NAME("getStaticDns2"))
            .returnUnsignedIntValueOrDefault(0u);
  }
};

#undef NAME
