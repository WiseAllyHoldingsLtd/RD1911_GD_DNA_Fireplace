#pragma once

#include <cstdint>


struct SpiTarget
{
  enum Enum {
    SystemTest = 1,
//    Radio = 2,
  };
};


class SpiDriverInterface
{
public:
  virtual void resetAndInit(void) = 0;
  virtual void sendReceive(SpiTarget::Enum target, uint8_t sendBuffer[], uint8_t receiveBuffer[], uint8_t bufferLength) = 0;
//  virtual bool sendAsync(uint8_t sendBuffer[], uint8_t bufferLength) = 0;
//  virtual bool receiveAsync(uint8_t receiveBuffer[], uint8_t bufferLength) = 0;

  virtual ~SpiDriverInterface() {}
};
