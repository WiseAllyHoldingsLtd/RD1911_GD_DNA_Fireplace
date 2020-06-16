#pragma once


class WifiDriverInterface
{
public:
  virtual bool resetAndInit(void) = 0;
  virtual bool configureConnection(const char ssID[], const char password[]) const = 0;
  virtual bool configureDynamicIp(void) = 0;

  /**
   * @remark
   * First part of dotted quad (a.b.c.d) is assumed to be at the lowest memory address.
   * I guess we could call that big endian. This applies to all parameters.
   */

  virtual bool configureStaticIp(uint32_t staticIp,
                                 uint32_t staticNetmask,
                                 uint32_t staticGateway,
                                 uint32_t staticDns1,
                                 uint32_t staticDns2) = 0;
  virtual bool connectToWlan(void) = 0;
  virtual void disconnectFromWlan(void) = 0;
  virtual bool isConnectedToWlan(void) const = 0;

  virtual ~WifiDriverInterface() {};
};
