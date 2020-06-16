#pragma once

#include "CppUTestExt\MockSupport.h"
#include "WifiDriverInterface.h"

#define NAME(method) "WifiDriverMock::" method


class WifiDriverMock : public WifiDriverInterface
{
public:
  virtual bool resetAndInit(void)
  {
    return mock().actualCall(NAME("resetAndInit")).returnBoolValueOrDefault(true);
  }

  virtual bool configureConnection(const char ssID[], const char password[]) const
  {
    return mock().actualCall(NAME("configureConnection"))
        .withStringParameter("ssID", ssID)
        .withStringParameter("password", password)
        .returnBoolValueOrDefault(true);
  }

  virtual bool configureDynamicIp(void)
  {
    return mock().actualCall(NAME("configureDynamicIp")).returnBoolValueOrDefault(true);
  }

  virtual bool configureStaticIp(uint32_t staticIp,
                                 uint32_t staticNetmask,
                                 uint32_t staticGateway,
                                 uint32_t staticDns1,
                                 uint32_t staticDns2)
  {
    return mock().actualCall(NAME("configureStaticIp"))
        .withUnsignedIntParameter("staticIp", staticIp)
        .withUnsignedIntParameter("staticNetmask", staticNetmask)
        .withUnsignedIntParameter("staticGateway", staticGateway)
        .withUnsignedIntParameter("staticDns1", staticDns1)
        .withUnsignedIntParameter("staticDns2", staticDns2)
        .returnBoolValueOrDefault(true);
  }

  virtual bool connectToWlan(void)
  {
    return mock().actualCall(NAME("connectToWlan")).returnBoolValueOrDefault(true);
  }

  virtual void disconnectFromWlan(void)
  {
    mock().actualCall(NAME("disconnectFromWlan"));
  }

  virtual bool isConnectedToWlan(void) const
  {
    return mock().actualCall(NAME("isConnectedToWlan")).returnBoolValueOrDefault(true);
  }
};
