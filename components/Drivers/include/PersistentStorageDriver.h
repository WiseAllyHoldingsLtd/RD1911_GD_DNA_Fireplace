#pragma once
#include "nvs_flash.h"
#include "nvs.h"

#include "Mutex.hpp"
#include "PersistentStorageDriverInterface.h"


class PersistentStorageDriver : public PersistentStorageDriverInterface
{
public:
  PersistentStorageDriver(void);

  virtual bool init(const char* partition, bool readOnly);
  virtual bool erase(const char* partition);

  virtual bool readValue(const char* id, uint32_t * value);
  virtual bool writeValue(const char* id, uint32_t value);

  virtual bool readString(const char* id, char * str, uint32_t size);
  virtual bool writeString(const char* id, const char * str);

  virtual bool readBlob(const char* id, uint8_t data[], uint32_t & size);
  virtual bool writeBlob(const char* id, const uint8_t data[], uint32_t size);

private:
  bool open(nvs_handle * handle) const;
  bool commit(nvs_handle handle) const;
  void close(nvs_handle handle) const;

  bool m_initialized;
  nvs_open_mode m_mode;
  const char* m_partition;
  Mutex m_lock;
};
