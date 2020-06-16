#pragma once

#include "driver/spi_master.h"

#include "SpiDriverInterface.h"


class SpiDriver : public SpiDriverInterface
{
public:
  SpiDriver();
  virtual ~SpiDriver();

  virtual void resetAndInit(void);
  virtual void sendReceive(SpiTarget::Enum target, uint8_t sendBuffer[], uint8_t receiveBuffer[], uint8_t bufferLength);
private:
  spi_device_handle_t m_sysTestDevice;

  void freeDevices();
};
