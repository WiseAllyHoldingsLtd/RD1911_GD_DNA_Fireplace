#pragma once
#include <stdint.h>


class PersistentStorageDriverInterface
{
public:

  virtual ~PersistentStorageDriverInterface(){};

  virtual bool init(const char* partition, bool readOnly) = 0;
  virtual bool erase(const char* partition) = 0;

  virtual bool readValue(const char* id, uint32_t * value) = 0;
  virtual bool writeValue(const char* id, uint32_t value) = 0;

  virtual bool readString(const char* id, char * str, uint32_t size) = 0;
  virtual bool writeString(const char* id, const char * str) = 0;

  virtual bool readBlob(const char* id, uint8_t data[], uint32_t & size) = 0;
  virtual bool writeBlob(const char* id, const uint8_t data[], uint32_t size) = 0;
};
