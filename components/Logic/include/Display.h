#pragma once

#include "DisplayDriverInterface.h"
#include "DisplayInterface.h"


class Display : public DisplayInterface
{
public:
  Display(DisplayDriverInterface& displayDriver);
  virtual void setup();
  virtual void drawScreen(const ViewModelInterface& viewModel);

private:
  DisplayDriverInterface& m_displayDriver;
};
