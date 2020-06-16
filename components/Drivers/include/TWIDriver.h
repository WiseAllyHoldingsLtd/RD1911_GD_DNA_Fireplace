#pragma once

#include <cstdint>

#include "Mutex.hpp"
#include "TWIDriverInterface.h"


class TWIDriver : public TWIDriverInterface
{
public:
  virtual void setup(uint32_t busFrequency);
  virtual bool send(uint8_t slaveAddress, uint8_t *buffer, uint8_t sendLength);
  virtual bool recieve(uint8_t slaveAddress, uint8_t *buffer, uint8_t receiveLength);
  virtual bool sendRecieve(uint8_t slaveAddress, uint8_t *buffer, uint8_t sendLength, uint8_t receiveLength);

  virtual ~TWIDriver();
private:
  Mutex m_lock;
  bool _send(uint8_t slaveAddress, uint8_t *buffer, uint8_t sendLength);
  bool _recieve(uint8_t slaveAddress, uint8_t *buffer, uint8_t receiveLength);
};
