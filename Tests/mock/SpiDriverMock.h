#pragma once

#include "CppUTestExt\MockSupport.h"
#include "SpiDriverInterface.h"


class SpiDriverMock : public SpiDriverInterface
{
public:
  virtual void resetAndInit(void)
  {
    mock().actualCall("resetAndInit");
  }

  virtual void sendReceive(SpiTarget::Enum target, uint8_t sendBuffer[], uint8_t receiveBuffer[], uint8_t bufferLength)
  {
    mock().actualCall("sendReceive")
      .withParameter("target", target)
      .withParameterOfType("sendBuffer", "sendBuffer", sendBuffer)
      .withOutputParameter("receiveBuffer", receiveBuffer)
      .withParameter("bufferLength", bufferLength);
  }

  virtual bool sendAsync(uint8_t sendBuffer[], uint8_t bufferLength)
  {
    return mock().actualCall("sendAsync")
        .withParameterOfType("sendBuffer", "sendBuffer", sendBuffer)
        .withParameter("bufferLength", bufferLength)
        .returnBoolValue();
  }

  virtual bool receiveAsync(uint8_t receiveBuffer[], uint8_t bufferLength)
  {
    return mock().actualCall("receiveAsync")
        .withOutputParameter("receiveBuffer", receiveBuffer)
        .withParameter("bufferLength", bufferLength)
        .returnBoolValue();
  }

};
