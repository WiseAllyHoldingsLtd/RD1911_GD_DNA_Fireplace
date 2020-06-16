#pragma once

#include "CppUTestExt\MockSupport.h"
#include "ViewModelInterface.h"

class SystemTestCommunicatorMock : public SystemTestCommunicatorInterface
{
public:
  virtual bool getButtonStatus(ButtonStatus &buttonStatus)
  {
    return mock().actualCall("getButtonStatusS").withOutputParameter("buttonStatus", &buttonStatus).returnBoolValue();
  }

  virtual bool isSimulatedStandbySwitchActive(void)
  {
    return mock().actualCall("getIsSimulatedStandbySwitchActive").returnBoolValue();
  }

  virtual void resetButtonStatus(void)
  {
    mock().actualCall("resetButtonStatusS");
  }

  virtual void sendViewModelInfo(const ViewModelInterface &viewModel)
  {
    mock().actualCall("sendViewModelInfo").withPointerParameter("viewModel", (void*)&viewModel);
  }

  virtual bool shouldClearSettings(void)
  {
    return mock().actualCall("shouldClearSettings").returnBoolValue();
  }
};
