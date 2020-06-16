#pragma once

#include "ViewModelInterface.h"


class DisplayInterface
{
public:
  virtual void setup(void) = 0;
  virtual void drawScreen(const ViewModelInterface & viewModel) = 0;

  virtual ~DisplayInterface() {};
};
