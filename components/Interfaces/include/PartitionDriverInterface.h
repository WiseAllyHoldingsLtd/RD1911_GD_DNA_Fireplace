#pragma once
#include <cstdint>

class PartitionDriverInterface
{
public:
  virtual const char * getBootPartitionName(void) const = 0;
  virtual const char * getRunningPartitionName(void) const = 0;
  virtual const char * getUpdatePartitionName(void) const = 0;

  virtual uint32_t getBootPartitionAddress(void) const = 0;
  virtual uint32_t getRunningPartitionAddress(void) const = 0;
  virtual uint32_t getUpdatePartitionAddress(void) const = 0;

  virtual ~PartitionDriverInterface(void) {}
};
