#pragma once

#include "CppUTestExt\MockSupport.h"
#include "TWIDriverInterface.h"

#define NAME(method) "TWIMock::" method


class TWIMock : public TWIDriverInterface
{
public:
  virtual void setup(uint32_t busFrequency)
  {
    mock().actualCall(NAME("setup")).withParameter("busFrequency", busFrequency);
  }
  virtual bool send(uint8_t slaveAddress, uint8_t *buffer, uint8_t sendLength)
  {
    return mock().actualCall(NAME("send"))
        .withParameter("slaveAddress", slaveAddress)
        .withParameterOfType("SendBuffer", "buffer", static_cast<void*>(buffer))
        .withParameter("sendLength", sendLength)
        .returnBoolValue();
  }
  virtual bool recieve(uint8_t slaveAddress, uint8_t *buffer, uint8_t receiveLength)
  {
    return mock().actualCall(NAME("receive"))
        .withParameter("slaveAddress", slaveAddress)
        .withOutputParameterOfType("ReceiveBuffer", "buffer", static_cast<void*>(buffer))
        .withParameter("receiveLength", receiveLength)
        .returnBoolValue();
  }
  virtual bool sendRecieve(uint8_t slaveAddress, uint8_t *buffer, uint8_t sendLength, uint8_t receiveLength)
  {
    return mock().actualCall(NAME("sendReceive"))
        .withParameter("slaveAddress", slaveAddress)
        .withParameterOfType("SendBuffer", "buffer", static_cast<void*>(buffer))
        .withOutputParameterOfType("ReceiveBuffer", "buffer", static_cast<void*>(buffer))
        .withParameter("sendLength", sendLength)
        .withParameter("receiveLength", receiveLength)
        .returnBoolValue();
  }
};
