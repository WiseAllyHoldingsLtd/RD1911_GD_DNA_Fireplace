#pragma once

#include <stdint.h>
#include "DateTime.h"

/* Screen enums, used to determine which screen the viewModel should show. */
struct Screen
{
  enum Enum
  {
    undefinedScreen,
    mainScreen,
    buttonTestScreen,
    swVersionScreen,
    factoryResetScreen,
    confirmFactoryResetScreen,
    buttonLockedScreen,
    buttonUnlockedScreen,
    bluetoothSetupScreen,
    firmwareUpgradeScreen
  };
};


/** Struct with non-persistent data that needs to be passed between controller and viewmodel.
*/
struct UIState
{
  Screen::Enum currentScreen;
  uint64_t currentScreenEnterTimestampMs;
  uint64_t timeSinceBootMs;
  uint64_t timestampLastActiveMs;
  bool isBluetoothConnected;
  bool isBluetoothActive;
  bool isWifiConnected;
  bool isAzureConnected;
  bool isScreenOff;
};
