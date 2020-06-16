#pragma once

#include <cstring>
#include <string>
#include <vector>
#include "CppUTestExt\MockSupport.h"
#include "PersistentStorageDriverInterface.h"


#define NAME(method) "PersistentStorageDriverMock::" method


class PersistentStorageDriverMock : public PersistentStorageDriverInterface
{
  virtual bool init(const char* partition, bool readOnly)
  {
    return mock().actualCall(NAME("init"))
        .withStringParameter("partition", partition)
        .withBoolParameter("readOnly", readOnly)
        .returnBoolValueOrDefault(true);
  }

  virtual bool erase(const char* partition)
  {
    return mock().actualCall(NAME("erase")).withStringParameter("partition", partition).returnBoolValueOrDefault(true);
  }

  virtual bool readValue(const char* id, uint32_t * value)
  {
    return mock().actualCall(NAME("readValue"))
        .withStringParameter("id", id)
        .withOutputParameter("value", value)
        .returnBoolValueOrDefault(true);
  }

  virtual bool writeValue(const char* id, uint32_t value)
  {
    return mock().actualCall(NAME("writeValue"))
        .withStringParameter("id", id)
        .withUnsignedIntParameter("value", value)
        .returnBoolValueOrDefault(true);
  }

  virtual bool readString(const char* id, char * str, uint32_t size)
  {
    return mock().actualCall(NAME("readString"))
        .withStringParameter("id", id)
        .withOutputParameter("str", str)
        .withUnsignedIntParameter("size", size)
        .returnBoolValueOrDefault(true);
  }

  virtual bool writeString(const char* id, const char * str)
  {
    return mock().actualCall(NAME("writeString"))
        .withStringParameter("id", id)
        .withStringParameter("str", str)
        .returnBoolValueOrDefault(true);
  }


  virtual bool readBlob(const char * id, uint8_t data[], uint32_t & size)
  {
    return mock().actualCall(NAME("readBlob"))
        .withStringParameter("id", id)
        .withOutputParameter("data", data)
        .withOutputParameter("size", &size)
        .returnBoolValueOrDefault(true);
  }

  virtual bool writeBlob(const char * id, const uint8_t data[], uint32_t size)
  {
    return mock().actualCall(NAME("writeBlob"))
        .withStringParameter("id", id)
        .withMemoryBufferParameter("data", data, size)
        .withUnsignedIntParameter("size", size)
        .returnBoolValueOrDefault(true);
  }
};

class PersistentStorageDriverDummy : public PersistentStorageDriverInterface
{
public:
  class KeyValueUint
  {
  public:
    std::string key;
    uint32_t value;
  };

  class KeyValueStr
  {
  public:
    std::string key;
    std::string value;
  };

  class KeyValueBlob
  {
  public:
    std::string key;
    std::vector<uint8_t> value;
  };

  PersistentStorageDriverDummy(void)
  {
  }

  virtual bool init(const char * partition, bool readOnly)
  {
    return true;
  }

  virtual bool erase(const char * partition)
  {
    return false;
  }

  virtual bool readValue(const char* id, uint32_t * value)
  {
    std::string key(id);
    uint32_t valueElements = m_valueStorage.size();
    bool wasFound = false;

    for (uint32_t i = 0U; i < valueElements; ++i)
    {
      if (m_valueStorage[i].key == key)
      {
        wasFound = true;
        *value = m_valueStorage[i].value;
        break;
      }
    }

    return wasFound;
  }

  virtual bool writeValue(const char* id, uint32_t value)
  {
    std::string key(id);
    uint32_t valueElements = m_valueStorage.size();
    bool wasFound = false;

    for (uint32_t i = 0U; i < valueElements; ++i)
    {
      if (m_valueStorage[i].key == key)
      {
        wasFound = true;
        m_valueStorage[i].value = value;
        break;
      }
    }

    if (!wasFound)
    {
      KeyValueUint newEntry;
      newEntry.key = key;
      newEntry.value = value;
      m_valueStorage.push_back(newEntry);
    }

    return true;
  }

  virtual bool readString(const char* id, char * str, uint32_t size)
  {
    std::string key(id);
    uint32_t stringElements = m_stringStorage.size();
    bool wasFound = false;

    for (uint32_t i = 0U; i < stringElements; ++i)
    {
      if (m_stringStorage[i].key == key)
      {
        if (m_stringStorage[i].value.length() < size)
        {
          wasFound = true;
          strncpy(str, m_stringStorage[i].value.c_str(), size);
        }

        break;
      }
    }

    return wasFound;
  }

  virtual bool writeString(const char* id, const char * str)
  {
    std::string key(id);
    uint32_t stringElements = m_stringStorage.size();
    bool wasFound = false;

    for (uint32_t i = 0U; i < stringElements; ++i)
    {
      if (m_stringStorage[i].key == key)
      {
        wasFound = true;
        m_stringStorage[i].value = std::string(str);
        break;
      }
    }

    if (!wasFound)
    {
      KeyValueStr newEntry;
      newEntry.key = key;
      newEntry.value = std::string(str);
      m_stringStorage.push_back(newEntry);
    }

    return true;
  }

  virtual bool readBlob(const char* id, uint8_t data[], uint32_t & size)
  {
    std::string key(id);
    uint32_t blobElements = m_blobStorage.size();
    bool wasFound = false;

    for (uint32_t i = 0U; i < blobElements; ++i)
    {
      if (m_blobStorage[i].key == key)
      {
        if (m_blobStorage[i].value.size() <= size)
        {
          wasFound = true;
          size = m_blobStorage[i].value.size();
          memcpy(data, &(m_blobStorage[i].value[0]), size);
        }
        else
        {
          size = 0U;
        }

        break;
      }
    }

    if (!wasFound)
    {
      size = 0U;
    }

    return wasFound;
  }

  virtual bool writeBlob(const char* id, const uint8_t data[], uint32_t size)
  {
    std::string key(id);
    uint32_t blobElements = m_blobStorage.size();
    uint32_t foundIndex = blobElements;

    for (uint32_t i = 0U; i < blobElements; ++i)
    {
      if (m_blobStorage[i].key == key)
      {
        foundIndex = i;
        m_blobStorage[i].value.clear();
        break;
      }
    }

    if (foundIndex == blobElements)
    {
      KeyValueBlob newEntry;
      newEntry.key = key;
      newEntry.value = std::vector<uint8_t>();
      m_blobStorage.push_back(newEntry);
    }

    // we now have an empty value-vector in m_blobStorage at index foundIndex
    for (uint32_t i = 0U; i < size; i++)
    {
      m_blobStorage[foundIndex].value.push_back(data[i]);
    }

    return true;
  }


private:
  std::vector<KeyValueUint> m_valueStorage;
  std::vector<KeyValueStr> m_stringStorage;
  std::vector<KeyValueBlob> m_blobStorage;
};

