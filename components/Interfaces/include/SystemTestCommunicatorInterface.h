#pragma once

#include "ButtonStatus.h"
#include "ViewModelInterface.h"

class SystemTestCommunicatorInterface
{
public:
  /**
   * Returns true if output parameter buttonStatus is updated with relevant simulated button presses.
   */
  virtual bool getButtonStatus(ButtonStatus &buttonStatus) = 0;

  /**
   * Returns true if the simulated standbySwitch is active.
   */
  virtual bool isSimulatedStandbySwitchActive(void) = 0;

  /**
   * Makes getButtonStatus() return no info about buttons previously pressed/held
   * until new (simulated) button events occur.
   */
  virtual void resetButtonStatus(void) = 0;

  /**
   * Sends View Model to PC (or to nothing if PC is not connected) via SPI for system testing.
   * Also possibly receives info from PC about button status to simulate at the same time.
   */
  virtual void sendViewModelInfo(const ViewModelInterface &viewModel) = 0;
  
  /**
   * Returns true if PC told us to clear all settings
   */
  virtual bool shouldClearSettings(void) = 0;

  virtual ~SystemTestCommunicatorInterface() {}
};
