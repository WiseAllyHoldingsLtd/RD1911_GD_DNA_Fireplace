#pragma once

#include "DisplayInterface.h"
#include "CppUTestExt\MockSupport.h"

class DisplayMock : public DisplayInterface
{
  virtual void setup(void)
  {
    mock().actualCall("setup");
  }

  virtual void drawScreen(const ViewModelInterface & viewModel)
  {
    mock().actualCall("drawScreen").withPointerParameter("viewModel", (void*)&viewModel);
  }
};
