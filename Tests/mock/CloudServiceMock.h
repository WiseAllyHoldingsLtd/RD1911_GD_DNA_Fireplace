
#pragma once

#include "CppUTestExt\MockSupport.h"
#include "CloudServiceInterface.h"

#define NAME(method) "CloudServiceMock::" method

class CloudServiceMock : public CloudServiceInterface
{
public:
  virtual bool isRunning() const
  {
    return mock().actualCall(NAME("isRunning"))
        .returnBoolValueOrDefault(true);
  }

  virtual uint8_t getCurrentHealth() const
  {
    return mock().actualCall(NAME("getCurrentHealth"))
        .returnUnsignedIntValueOrDefault(0u);
  }

  virtual bool requestPause()
  {
    return mock().actualCall(NAME("requestPause"))
        .returnBoolValueOrDefault(true);
  }

  virtual bool requestUnpause()
  {
    return mock().actualCall(NAME("requestUnpause"))
        .returnBoolValueOrDefault(true);
  }

  virtual bool isPaused() const
  {
    return mock().actualCall(NAME("isPaused"))
        .returnBoolValueOrDefault(true);
  }

  virtual void waitUntilPausedOrUnpaused(bool waitForPaused)
  {
    mock().actualCall(NAME("waitUntilPausedOrUnpaused")).withBoolParameter("waitForPaused", waitForPaused);
  }

  virtual bool wasDiscoverySuccessfulSinceLastRestart() const
  {
    return mock().actualCall(NAME("wasDiscoverySuccessfulSinceLastRestart"))
        .returnBoolValueOrDefault(false);
  }
};

#undef NAME
