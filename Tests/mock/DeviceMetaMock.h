#pragma once

#include "CppUTestExt\MockSupport.h"
#include "DeviceMetaInterface.h"

#define NAME(method) "DeviceMetaMock::" method


class DeviceMetaMock : public DeviceMetaInterface
{
public:
  virtual void getGDID(char * gdid, uint32_t size)
  {
    mock().actualCall(NAME("getGDID"))
        .withOutputParameter("gdid", static_cast<void*>(gdid))
        .withUnsignedIntParameter("size", size);
  }

  virtual void getGDIDAsBCD(uint8_t (&gdid)[Constants::GDID_BCD_SIZE])
  {
    mock().actualCall(NAME("getGDIDAsBCD")).withOutputParameter("gdid", static_cast<void*>(gdid));
  }

  virtual void setGDID(const char * gdid)
  {
    mock().actualCall(NAME("setGDID")).withStringParameter("gdid", gdid);
  }

  virtual uint32_t getBluetoothPasskey(void)
  {
    return mock().actualCall(NAME("getBluetoothPasskey")).returnUnsignedIntValue();
  }

  virtual void setBluetoothPasskey(uint32_t key)
  {
    mock().actualCall(NAME("setBluetoothPasskey")).withUnsignedIntParameter("key", key);
  }

  virtual EOLStatus::Enum getEndOfLineStatus(void)
  {
    return static_cast<EOLStatus::Enum>(mock().actualCall(NAME("getEndOfLineStatus")).returnIntValue());
  }

  virtual void setEndOfLineStatus(EOLStatus::Enum eolStatus)
  {
    mock().actualCall(NAME("setEndOfLineStatus")).withIntParameter("eolStatus", static_cast<int>(eolStatus));
  }
};
