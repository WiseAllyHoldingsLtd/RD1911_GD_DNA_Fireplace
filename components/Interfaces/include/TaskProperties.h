#pragma once
#include <cstdint>

struct TaskPriority
{
  enum Enum : uint8_t
  {
    /* Used to ensure that 'our' task doesn't have equal priority, as might be a problem (at least it could be in earlier esp-idf versions) */
    statusDebug = 1U,
    mainTask,
    heatController,
    connectionController,
    azureService,
    cloudService,
    fwUpgrade,
    buttonDriver,
    userInterface,
  };
};

struct TaskStackSize
{
  static const uint16_t statusDebug = 1536U;
  static const uint16_t mainTask = (12U * 1024U);
  static const uint16_t heatController = 2048U;
  static const uint16_t connectionController = 2048U;
  static const uint16_t cloudService = (10U * 1024U);
  static const uint16_t azureService = (10U * 1024U);
  static const uint16_t fwUpgrade = (16U * 1024U);
  static const uint16_t buttonDriver = 1024U;
  static const uint16_t userInterface = 2048U;
};
