#pragma once

#include "Task.hpp"

#include "CloudDriverInterface.h"
#include "DeviceMetaInterface.h"
#include "SettingsInterface.h"
#include "DiscoveryServiceInterface.h"
#include "EventFlagsInterface.hpp"
#include "WifiConnectionStatus.h"
#include "QueueInterface.hpp"
#include "CloudServiceInterface.h"
#include "SntpDriverInterface.h"

#include "HealthCheckService.hpp"

#include "CloudServiceCommand.h"

class CloudServiceTask : public Task, public CloudServiceInterface
{
public:
  CloudServiceTask();
  void init(const CloudDriverInterface *cloud,
            DeviceMetaInterface *deviceMeta,
            SettingsInterface *settings,
            QueueInterface<ConnectionStrings_t> *queue,
            QueueInterface<WifiConnectionStatus::Enum> *discoveryResultQueue,
            EventFlagsInterface *healthCheckEvents,
            QueueInterface<CloudServiceCommand> *commandQueue,
            SntpDriverInterface *sntpDriver);
  virtual void run(bool eternalLoop);
  bool isRunning() const;
  uint8_t getCurrentHealth() const;
  bool requestPause();
  bool requestUnpause();
  bool isPaused() const;
  void waitUntilPausedOrUnpaused(bool waitForPaused);
  bool wasDiscoverySuccessfulSinceLastRestart() const;
private:
  bool isControlCmdReceived(uint32_t timeout,
                            CloudServiceCommand &command);
  bool isUnpauseRequested(uint32_t timeout);
  bool isPauseRequested(uint32_t timeout);
  void setWasDiscoverySuccessfulSinceLastRestart(bool wasDiscoverySuccessfulSinceLastRestart);
  void checkHealth(HealthCheckService &healthCheckService);
  bool shouldRunRegularHealthChecks();

  const CloudDriverInterface *m_cloudDriver;
  DeviceMetaInterface *m_deviceMeta;
  SettingsInterface *m_settings;
  QueueInterface<ConnectionStrings_t> *m_connectionStringQueue;
  QueueInterface<WifiConnectionStatus::Enum> *m_discoveryResultQueue;
  EventFlagsInterface *m_healthCheckEventFlags;
  volatile bool m_running;
  volatile bool m_paused;
  bool m_inited;
  volatile uint8_t m_currentHealth;
  QueueInterface<CloudServiceCommand> *m_commandQueue;
  SntpDriverInterface *m_sntpDriver;
  volatile bool m_wasDiscoverySuccessfulSinceLastRestart;
};
