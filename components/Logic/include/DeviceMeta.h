#pragma once

#include "Mutex.hpp"
#include "Constants.h"
#include "DeviceMetaInterface.h"
#include "DeviceMetaStorageInterface.h"


class DeviceMeta : public DeviceMetaInterface
{
public:
  DeviceMeta(DeviceMetaStorageInterface & storage);
  void retrieveMeta(void);
  void storeMeta(void);

  virtual void getGDID(char * gdid, uint32_t size);
  virtual void getGDIDAsBCD(uint8_t (&gdid)[Constants::GDID_BCD_SIZE]);
  virtual void setGDID(const char * gdid);

  virtual uint32_t getBluetoothPasskey(void);
  virtual void setBluetoothPasskey(uint32_t key);

  virtual EOLStatus::Enum getEndOfLineStatus(void);
  virtual void setEndOfLineStatus(EOLStatus::Enum eolStatus);

  bool isModified(void) const;

private:
  void setDefaultMeta(void);

  Mutex m_accessorLock;
  Mutex m_storeRetrieveLock;

  char m_GDID[Constants::GDID_SIZE_MAX];
  uint32_t m_btPasskey;
  EOLStatus::Enum m_eolStatus;
  bool m_isModified;

  DeviceMetaStorageInterface & m_storage;
};
