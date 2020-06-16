#pragma once
#include "PartitionDriverInterface.h"

class PartitionDriver : public PartitionDriverInterface
{
public:
  PartitionDriver(void);

  virtual const char * getBootPartitionName(void) const;
  virtual const char * getRunningPartitionName(void) const;
  virtual const char * getUpdatePartitionName(void) const;

  virtual uint32_t getBootPartitionAddress(void) const;
  virtual uint32_t getRunningPartitionAddress(void) const;
  virtual uint32_t getUpdatePartitionAddress(void) const;
};
