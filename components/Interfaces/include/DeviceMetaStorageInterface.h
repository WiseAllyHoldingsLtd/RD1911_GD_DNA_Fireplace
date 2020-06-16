#pragma once

#include <stdint.h>
#include "EOLStatus.h"


/* Unique keys used for storing device meta data in NonVolatile storage */
class MetaUID
{
public:
  static const char GDID[];
  static const char BT_PASSKEY[];
  static const char EOL_STATUS[];
};


class DeviceMetaStorageInterface
{
public:
  virtual ~DeviceMetaStorageInterface(void) {}

  virtual bool retrieveGDID(char * gdid, uint32_t size) const = 0;
  virtual bool storeGDID(const char * gdid) = 0;

  virtual bool retrieveBluetoothPasskey(uint32_t & passkey) const = 0;
  virtual bool storeBluetoothPasskey(uint32_t passkey) = 0;

  virtual bool retrieveEOLStatus(EOLStatus::Enum & eolStatus) const = 0;
  virtual bool storeEOLStatus(EOLStatus::Enum eolStatus) = 0;
};
