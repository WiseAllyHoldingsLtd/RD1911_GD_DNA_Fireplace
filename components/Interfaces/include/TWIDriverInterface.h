#pragma once

#include <cstdint>


class TWIDriverInterface
{
public:
  virtual void setup(uint32_t busFrequency) = 0;
  virtual bool send(uint8_t slaveAddress, uint8_t *buffer, uint8_t sendLength) = 0;
  virtual bool recieve(uint8_t slaveAddress, uint8_t *buffer, uint8_t receiveLength) = 0;
  virtual bool sendRecieve(uint8_t slaveAddress, uint8_t *buffer, uint8_t sendLength, uint8_t receiveLength) = 0;

  virtual ~TWIDriverInterface() {};
};
