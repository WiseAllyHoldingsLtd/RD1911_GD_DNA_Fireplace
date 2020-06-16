#pragma once

#include "QueueInterface.hpp"
#include "DeviceMetaInterface.h"
#include "CloudDriverInterface.h"
#include "DiscoveryServiceInterface.h"
#include "WifiConnectionStatus.h"

class DiscoveryService : public DiscoveryServiceInterface
{
public:
  DiscoveryService(const CloudDriverInterface &cloud,
                   DeviceMetaInterface &meta,
                   QueueInterface<ConnectionStrings_t> &queue,
                   QueueInterface<WifiConnectionStatus::Enum> &resultQueue);
  virtual bool registerDevice(); // primary & secondary here?

private:
  const CloudDriverInterface &m_cloudDriver;
  DeviceMetaInterface &m_deviceMeta;
  QueueInterface<ConnectionStrings_t> &m_connectionStringQueue;
  QueueInterface<WifiConnectionStatus::Enum> &m_resultQueue;
  char m_buffer[512]; // FIXME: constant?
};
