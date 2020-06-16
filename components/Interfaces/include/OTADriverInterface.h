#pragma once
#include <stdint.h>

class OTADriverInterface
{
public:

  virtual ~OTADriverInterface(){};

  /**
  * Resets and initializes OTADriver. If an update has been installed and marked as active, the partition to boot from
  * is set back to the original boot partition. Any changes written to the 'update' partition is NOT reverted.
  */
  virtual void resetAndInit(void) = 0;

  /**
   * Performs a self test by checking that partition table has required partitions for OTA update functionality.
   */
  virtual bool performSelfTest(void) const = 0;

  /**
  * Allocates memory for writing and prepares the "update" partition (i.e. erases current contents). Memory must be
  * freed by calling endUpdate().
  */
  virtual bool beginUpdate(void) = 0;

  /**
   * Validates new image, and releases memory allocated by beginUpdate().
   */
  virtual bool endUpdate(void) = 0;

  /**
   * Sequentially writes chunk of data to the update partition. This function can be called multiple times.
   */
  virtual bool write(const char * data, uint32_t size) = 0;

  /**
   * Marks the "update" partition as boot partition. Thus, on next boot the device runs from the newly installed FW.
   */
  virtual bool activateUpdatedPartition(void) = 0;
};

