#pragma once

#include "TaskProperties.h"
#include "Task.hpp"
#include <esp_partition.h>

class StatusTask : public AutoStartingTask
{
public:
  StatusTask(void)
    : AutoStartingTask("StatusDbg", TaskStackSize::statusDebug, TaskPriority::statusDebug) {}

  virtual void run(bool eternalLoop);

private:
  void printChipInfo(void) const;
  void printIDFVersion(void) const;
  void printRunningPartitionInfo(void) const;
  void printPartitionTable(esp_partition_type_t type) const;
  void printHeapInformation(void) const;
  void printTaskStatus(void) const;

  unsigned int getPercentage(unsigned int number, unsigned int total) const;
  char getTaskStateCode(eTaskState state) const;
  char getPartitionSubTypeCode(esp_partition_type_t type, esp_partition_subtype_t subType) const;

};
