#pragma once

#include "Task.hpp"
#include "RUInterface.h"
#include "TimerDriverInterface.h"
#include "SystemTimeDriverInterface.h"
#include "SettingsInterface.h"
#include "QueueInterface.hpp"
#include "TWIDriverInterface.h"
#include "SoftwareResetDriverInterface.h"
#include "WeekProgramNodes.h"


class HeatControllerTask : public Task
{
public:
  HeatControllerTask(RUInterface &regUnit, TimerDriverInterface &timer, SystemTimeDriverInterface &systemTime, SettingsInterface &settings,
                      QueueInterface<bool> &isHeatElementOnQueue, TWIDriverInterface &twiDriver, SoftwareResetDriverInterface &resetDriver);

  virtual void run(bool eternalLoop);

private:
  bool updateRUSetPoint(int32_t setPoint);
  void handleRUCommFailures(void);
  void clearOverrideIfNow(const Override & override);
  void clearOverrideIfTimeExpired(const Override & override, const DateTime &currentDateTime);

  RUInterface &m_regUnit;
  TimerDriverInterface &m_timerDriver;
  SystemTimeDriverInterface &m_systemTime;
  SettingsInterface &m_settings;
  QueueInterface<bool> &m_isRUHeatElementOnQueue;
  TWIDriverInterface &m_twiDriver;
  SoftwareResetDriverInterface &m_resetDriver;

  /*
   * Adding these as members to allow for multiple calls to run() during unit tests.
   */
  int32_t m_setPoint;
  uint8_t m_ruCommFailures;
  uint64_t m_timestampLastRUHeatElementUpdate;
  uint64_t m_timestampLastConfigResend;
  WeekProgramNodes m_weekProgramNodes;
};













































