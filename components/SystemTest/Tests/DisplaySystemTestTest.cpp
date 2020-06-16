#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "DisplaySystemTest.h"
#include "ViewModelMock.h"
#include "DisplayMock.h"
#include "SystemTestCommunicatorMock.h"


TEST_GROUP(displaySystemTest)
{
  TEST_SETUP()
  {
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};

TEST(displaySystemTest, testCallsSystemTestCommunicator)
{
  SystemTestCommunicatorMock sysTestComMock;
  DisplayMock displayMock;
  DisplaySystemTest displaySystemTest(sysTestComMock, displayMock);
  ViewModelMock viewModelMock;

  mock().expectOneCall("sendViewModelInfo").withPointerParameter("viewModel", &viewModelMock);
  mock().ignoreOtherCalls();

  displaySystemTest.drawScreen(viewModelMock);
}


TEST(displaySystemTest, testCallsRealDisplay)
{
  SystemTestCommunicatorMock sysTestComMock;
  DisplayMock displayMock;
  DisplaySystemTest displaySystemTest(sysTestComMock, displayMock);
  ViewModelMock viewModelMock;

  mock().expectOneCall("drawScreen").withPointerParameter("viewModel", &viewModelMock);
  mock().ignoreOtherCalls();

  displaySystemTest.drawScreen(viewModelMock);
}
