#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"
#include "UIController.h"
#include "UIState.h"

static UIController* controller;
static UIState uiState;

TEST_GROUP(uiController)
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


TEST(uiController, test)
{
  //We decided to not make any automated unit tests for UIController, as that would require relatively much work.
}
