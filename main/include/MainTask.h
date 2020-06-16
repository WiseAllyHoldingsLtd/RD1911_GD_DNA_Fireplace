
#pragma once

#include "Task.hpp"

class MainTask : public Task
{
public:
  virtual void run(bool eternalLoop);
private:
};
