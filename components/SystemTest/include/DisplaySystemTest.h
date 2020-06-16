#pragma once

#include "DisplayInterface.h"
#include "SystemTestCommunicatorInterface.h"


class DisplaySystemTest : public DisplayInterface
{
public:
  DisplaySystemTest(SystemTestCommunicatorInterface & systemTestCommunicator, DisplayInterface & realDisplay);
  virtual void setup();
  virtual bool performSelfTest();
  virtual void drawScreen(const ViewModelInterface & viewModel);

private:
  DisplayInterface & m_realDisplay;
  SystemTestCommunicatorInterface & m_systemTestCommunicator;
};
