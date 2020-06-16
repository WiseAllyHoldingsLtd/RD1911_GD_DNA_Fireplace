
#pragma once

#include "Constants.h"
#include "CloudStatus.hpp"

class AzureDriverInterface
{
public:
  virtual ~AzureDriverInterface() { }

  virtual bool isConnected() const = 0;
  virtual bool isIdle() const = 0;
  virtual CloudStatus::Enum getStatus() const = 0;
  virtual void requestShutdown() = 0;
  virtual bool isUnableToSend() const = 0;

  virtual bool connect(const char (&connectionString)[Constants::CLOUD_CONNECTION_STRING_MAX_LENGTH]) = 0;
};
