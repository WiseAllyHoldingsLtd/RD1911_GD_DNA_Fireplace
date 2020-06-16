#include "ButtonSystemTestDriver.h"
#include "SystemTestCommunicatorInterface.h"
#include "ButtonDriverInterface.h"


ButtonSystemTestDriver::ButtonSystemTestDriver(SystemTestCommunicatorInterface &systemTestCommunicator,
                                               ButtonDriverInterface &realButtonDriver) :
                                               m_realButtonDriver(realButtonDriver),
                                               m_systemTestCommunicator(systemTestCommunicator)
{
}

ButtonStatus ButtonSystemTestDriver::getButtonStatus(void)
{
  ButtonStatus buttonStatus;

  if (!m_systemTestCommunicator.getButtonStatus(buttonStatus))
  {
    buttonStatus = m_realButtonDriver.getButtonStatus();
  }
  return buttonStatus;
}

void ButtonSystemTestDriver::resetButtonStatus(void)
{
  m_systemTestCommunicator.resetButtonStatus();

  m_realButtonDriver.resetButtonStatus();
}

