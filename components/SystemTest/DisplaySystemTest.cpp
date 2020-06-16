#include "DisplaySystemTest.h"
#include "SystemTestCommunicatorInterface.h"
#include "DisplayInterface.h"


DisplaySystemTest::DisplaySystemTest(SystemTestCommunicatorInterface & systemTestCommunicator,
                                     DisplayInterface & realDisplay) :
                                     m_realDisplay(realDisplay),
                                     m_systemTestCommunicator(systemTestCommunicator)
{
}

void DisplaySystemTest::setup()
{

}

bool DisplaySystemTest::performSelfTest()
{
  return true;
}

void DisplaySystemTest::drawScreen(const ViewModelInterface & viewModel)
{
  m_systemTestCommunicator.sendViewModelInfo(viewModel);

  m_realDisplay.drawScreen(viewModel);
}

