#pragma once

#include "QueueInterface.hpp"
#include "EventFlagsInterface.hpp"
#include "DeviceMetaInterface.h"
#include "SettingsInterface.h"
#include "CloudDriverInterface.h"

class HealthCheckService
{
public:
  HealthCheckService(const CloudDriverInterface &cloud,
                     DeviceMetaInterface &meta,
                     SettingsInterface &settings,
                     EventFlagsInterface &eventFlags);
  bool checkHealth(uint8_t &health);

private:
  const CloudDriverInterface &m_cloudDriver;
  DeviceMetaInterface &m_deviceMeta;
  SettingsInterface &m_settings;
  EventFlagsInterface &m_eventFlags;
//  QueueInterface<some struct> &m_timeSyncQueue;
  char m_buffer[256]; // FIXME: constant?
};
