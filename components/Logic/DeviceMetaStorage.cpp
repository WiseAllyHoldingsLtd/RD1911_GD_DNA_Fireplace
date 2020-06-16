#include "DeviceMetaStorage.h"


DeviceMetaStorage::DeviceMetaStorage(PersistentStorageDriverInterface & driver)
: m_driver(driver)
{
}


bool DeviceMetaStorage::retrieveGDID(char * gdid, uint32_t size) const
{
  return m_driver.readString(MetaUID::GDID, gdid, size);
}


bool DeviceMetaStorage::storeGDID(const char * gdid)
{
  return m_driver.writeString(MetaUID::GDID, gdid);
}


bool DeviceMetaStorage::retrieveBluetoothPasskey(uint32_t & passkey) const
{
  return m_driver.readValue(MetaUID::BT_PASSKEY, &passkey);
}


bool DeviceMetaStorage::storeBluetoothPasskey(uint32_t passkey)
{
  return m_driver.writeValue(MetaUID::BT_PASSKEY, passkey);
}


bool DeviceMetaStorage::retrieveEOLStatus(EOLStatus::Enum & eolStatus) const
{
  uint32_t data;
  bool result = m_driver.readValue(MetaUID::EOL_STATUS, &data);

  if (result)
  {
    eolStatus = static_cast<EOLStatus::Enum>(data);
  }

  return result;
}


bool DeviceMetaStorage::storeEOLStatus(EOLStatus::Enum eolStatus)
{
  uint32_t data = static_cast<uint32_t>(eolStatus);
  return m_driver.writeValue(MetaUID::EOL_STATUS, data);
}
