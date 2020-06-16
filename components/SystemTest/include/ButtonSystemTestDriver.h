#pragma once

#include "ButtonDriverInterface.h"
#include "SystemTestCommunicatorInterface.h"

class ButtonSystemTestDriver : public ButtonDriverInterface
{
public:
  ButtonSystemTestDriver(SystemTestCommunicatorInterface &systemTestCommunicator, ButtonDriverInterface &realButtonDriver);
  virtual ButtonStatus getButtonStatus(void);
  virtual void resetButtonStatus(void);

private:
  ButtonDriverInterface &m_realButtonDriver;
  SystemTestCommunicatorInterface & m_systemTestCommunicator;
};
