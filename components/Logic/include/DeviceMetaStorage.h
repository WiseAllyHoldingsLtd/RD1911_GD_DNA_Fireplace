#pragma once

#include "DeviceMetaStorageInterface.h"
#include "PersistentStorageDriverInterface.h"


class DeviceMetaStorage : public DeviceMetaStorageInterface
{
public:
  DeviceMetaStorage(PersistentStorageDriverInterface &driver);

  virtual bool retrieveGDID(char * gdid, uint32_t size) const;
  virtual bool storeGDID(const char * gdid);

  virtual bool retrieveBluetoothPasskey(uint32_t & passkey) const;
  virtual bool storeBluetoothPasskey(uint32_t passkey);

  virtual bool retrieveEOLStatus(EOLStatus::Enum & eolStatus) const;
  virtual bool storeEOLStatus(EOLStatus::Enum eolStatus);

private:
  PersistentStorageDriverInterface & m_driver;
};
