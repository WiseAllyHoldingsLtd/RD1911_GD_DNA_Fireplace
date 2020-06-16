#pragma once

#include <cstdint>

struct ConnectionState
{
  enum Enum : uint8_t
  {
    NotConnected = 0u, // NOTE: don't change indexing as NUM_STATES depends on zero-based index
    WifiConnecting,
    WifiConnected,
    DiscoveryConnecting,
    DiscoveryConnected,
    IotHubConnecting,
    IotHubConnected,
    ActivatingWifiSetup,
    WaitingForBleOff,
    FwUpgradeInProgress,
    FactoryReset,

    NUM_STATES
  };
};


class ConnectionControllerTaskInterface
{
public:
  virtual ~ConnectionControllerTaskInterface() { }

  virtual bool startTask() = 0;
  virtual ConnectionState::Enum getState(void) const = 0;
};
