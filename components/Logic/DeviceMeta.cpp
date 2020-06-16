#include <string.h>
#include "DeviceMeta.h"


DeviceMeta::DeviceMeta(DeviceMetaStorageInterface & storage)
 : m_storage(storage)
{
  setDefaultMeta();
  m_isModified = false;
}


void DeviceMeta::setDefaultMeta(void)
{
  setGDID(Constants::DEFAULT_GDID);
  m_btPasskey = Constants::DEFAULT_BT_PASSKEY;
  m_eolStatus = Constants::DEFAULT_EOL_STATUS;
  m_isModified = true;
}


void DeviceMeta::retrieveMeta(void)
{
  m_storeRetrieveLock.take();
  char gdid[Constants::GDID_SIZE_MAX];
  uint32_t btKey;
  EOLStatus::Enum eolStatus;

  if (m_storage.retrieveGDID(gdid, sizeof(gdid)))
  {
    setGDID(gdid);
  }

  if (m_storage.retrieveBluetoothPasskey(btKey))
  {
    setBluetoothPasskey(btKey);
  }

  if (m_storage.retrieveEOLStatus(eolStatus))
  {
    setEndOfLineStatus(eolStatus);
  }

  m_isModified = false;
  m_storeRetrieveLock.give();
}


void DeviceMeta::storeMeta(void)
{
  m_storeRetrieveLock.take();
  m_storage.storeGDID(m_GDID);
  m_storage.storeBluetoothPasskey(getBluetoothPasskey());
  m_storage.storeEOLStatus(m_eolStatus);

  m_isModified = false;
  m_storeRetrieveLock.give();
}


void DeviceMeta::getGDID(char * gdid, uint32_t size)
{
  m_accessorLock.take();
  strncpy(gdid, m_GDID, size);
  gdid[size - 1U] = '\0';
  m_accessorLock.give();
}


void DeviceMeta::getGDIDAsBCD(uint8_t (&gdid)[Constants::GDID_BCD_SIZE])
{
  // Returns GDID as BCD. Assuming LSB first
  memset(gdid, 0, Constants::GDID_BCD_SIZE);
  uint8_t digitsInGDID = strnlen(m_GDID, Constants::GDID_SIZE_MAX - 1U);

  for (uint8_t i = 0U; i < digitsInGDID; ++i)
  {
    uint8_t byteNo = ((digitsInGDID - i - 1U) / 2U);
    uint8_t value = 0U;

    if ((m_GDID[i] >= '0') && (m_GDID[i] <= '9'))
    {
      value = m_GDID[i] - '0';
    }
    else
    {
      value = (m_GDID[i] - 'A' + 10U);
    }

    if ((i % 2) == (digitsInGDID % 2)) // upper part of byte
    {
      gdid[byteNo] |= (value << 4U);
    }
    else // lower part of byte
    {
      gdid[byteNo] |= value;
    }
  }
}


void DeviceMeta::setGDID(const char * gdid)
{
  m_accessorLock.take();
  strncpy(m_GDID, gdid, sizeof(m_GDID));
  m_GDID[sizeof(m_GDID) - 1U] = '\0';

  m_isModified = true;
  m_accessorLock.give();
}


uint32_t DeviceMeta::getBluetoothPasskey(void)
{
  return m_btPasskey;
}


void DeviceMeta::setBluetoothPasskey(uint32_t key)
{
  m_btPasskey = key;
  m_isModified = true;
}


EOLStatus::Enum DeviceMeta::getEndOfLineStatus(void)
{
  return m_eolStatus;
}


void DeviceMeta::setEndOfLineStatus(EOLStatus::Enum eolStatus)
{
  switch (eolStatus)
  {
  case EOLStatus::notCompleted:
  case EOLStatus::completed:
    break;
  default:
    eolStatus = Constants::DEFAULT_EOL_STATUS;
    break;
  }

  m_eolStatus = eolStatus;
  m_isModified = true;
}


bool DeviceMeta::isModified(void) const
{
  return m_isModified;
}
