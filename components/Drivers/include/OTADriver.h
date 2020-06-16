#pragma once

#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "OTADriverInterface.h"


class OTADriver : public OTADriverInterface
{
public:
  OTADriver(void);
  virtual void resetAndInit(void);
  virtual bool performSelfTest(void) const;
  virtual bool beginUpdate(void);
  virtual bool endUpdate(void);
  virtual bool write(const char * data, uint32_t size);
  virtual bool activateUpdatedPartition(void);

private:
  bool isOTAEnabled(void) const;
  bool isUpdateOngoing(void) const;

  const esp_partition_t * const m_originalBootPartition;
  const esp_partition_t * const m_updateTargetPartition;
  esp_ota_handle_t m_updateHandle;
  bool m_isUpdateCompleted;
  bool m_isUpdateActivated;
};
