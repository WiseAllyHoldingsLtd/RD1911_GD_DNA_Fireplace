#pragma once
#include <cstdint>


class SystemResourceDriver
{
public:
  uint32_t getCurrentFreeHeap(void);
  uint32_t getMinimumFreeHeapSinceBoot(void);
};
