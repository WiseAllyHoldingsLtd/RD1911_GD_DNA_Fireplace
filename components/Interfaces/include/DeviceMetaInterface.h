#pragma once

#include <stdint.h>
#include "EOLStatus.h"
#include "Constants.h"

class DeviceMetaInterface
{
public:
  virtual ~DeviceMetaInterface(){};

  virtual void getGDID(char * gdid, uint32_t size) = 0;
  virtual void getGDIDAsBCD(uint8_t (&gdid)[Constants::GDID_BCD_SIZE]) = 0;
  virtual void setGDID(const char * gdid) = 0;

  virtual uint32_t getBluetoothPasskey(void) = 0;
  virtual void setBluetoothPasskey(uint32_t key) = 0;

  virtual EOLStatus::Enum getEndOfLineStatus(void) = 0;
  virtual void setEndOfLineStatus(EOLStatus::Enum eolStatus) = 0;
};
