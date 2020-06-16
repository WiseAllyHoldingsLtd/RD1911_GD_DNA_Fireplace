#pragma once
#include <esp_err.h>
#include <esp_event.h>
#include <esp_wifi_types.h>
#include "EventFlags.hpp"
#include "WifiDriverInterface.h"

class WifiDriver : public WifiDriverInterface
{
public:
  WifiDriver(void);
  virtual bool resetAndInit(void);
  virtual bool configureConnection(const char ssID[], const char password[]) const;
  virtual bool configureDynamicIp();
  virtual bool configureStaticIp(uint32_t staticIp,
                                 uint32_t staticNetmask,
                                 uint32_t staticGateway,
                                 uint32_t staticDns1,
                                 uint32_t staticDns2);

  virtual bool connectToWlan(void);
  virtual void disconnectFromWlan(void);
  virtual bool isConnectedToWlan(void) const;

private:
  static esp_err_t eventHandler(void * context, system_event_t * event);
  const static uint8_t CONNECTING_EVENT_FINISHED = 0U;

  void reset(void);
  bool init(void);

  bool m_isInitialized;
  bool m_isEventLoopStarted;
  bool m_isWlanConnected;
  EventFlags m_eventFlags;
};
