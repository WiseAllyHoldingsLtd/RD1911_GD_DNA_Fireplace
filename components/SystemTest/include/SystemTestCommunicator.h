#pragma once

#include <cstdint>

#include "SystemTestCommunicatorInterface.h"
#include "SpiDriverInterface.h"


/**
* Communications protocol is documented here:
* http://buildserver-01.orion.local:8095/display/DTB2R/API+for+System+Test+communication+NTB-2R+%3C-%3E+PC%2C+via+SPI
*/
class SystemTestCommunicator : public SystemTestCommunicatorInterface
{
public:
  SystemTestCommunicator(SpiDriverInterface &spiDriver);
  virtual bool getButtonStatus(ButtonStatus &buttonStatus);
  virtual void resetButtonStatus(void);
  virtual bool isSimulatedStandbySwitchActive(void);
  virtual void sendViewModelInfo(const ViewModelInterface &viewModel);
  virtual bool shouldClearSettings(void);

private:
  void serialiseViewModel(uint8_t sendBuffer[], const ViewModelInterface &viewModel);

  /**
   * Returns true if new Button Message is received and parsed successfully.
   */
  bool deserialiseButtonStatus(uint8_t receiveBuffer[], ButtonStatus &newButtonStatus);
  
  bool deserialiseClearSettings(uint8_t receiveBuffer[], bool &clearSettings);

  SpiDriverInterface &m_spiDriver;
  ButtonStatus m_simulatedButtonStatus;
  bool m_isSimulatedButtonStatusActive;
  bool m_shouldClearSettings;
  bool m_isSimulatedStandbySwitchStatus;
  bool m_isButtonReset;
};
