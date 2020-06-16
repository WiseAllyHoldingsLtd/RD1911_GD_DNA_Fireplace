#pragma once

#include <cstdint>
#include "PartitionDriverInterface.h"
#include "TimerDriverInterface.h"
#include "DisplayInterface.h"
#include "RtcInterface.h"
#include "SystemTimeDriverInterface.h"
#include "RUInterface.h"
#include "PersistentStorageDriverInterface.h"
#include "CryptoDriverInterface.h"
#include "WifiDriverInterface.h"
#include "BTDriverInterface.h"

class PowerOnSelfTest
{
public:
  PowerOnSelfTest(void);
  bool run(PartitionDriverInterface &partitions, TimerDriverInterface &timer, DisplayInterface &display,
            RtcInterface &rtc, SystemTimeDriverInterface &sysTime,
            RUInterface &ru, CryptoDriverInterface &crypto,
            BTDriverInterface &bluetooth, WifiDriverInterface &wifi,
            PersistentStorageDriverInterface &settingsStorage,
            PersistentStorageDriverInterface &metaStorage);

  int32_t getErrorCode(void) const;
  bool isFatalError(void) const;

private:
  int32_t m_errorCode;
};
