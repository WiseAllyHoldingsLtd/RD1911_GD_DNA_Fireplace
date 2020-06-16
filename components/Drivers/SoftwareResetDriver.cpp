#include "esp_system.h"
#include "SoftwareResetDriver.h"


SoftwareResetDriver::SoftwareResetDriver(void)
{
}

void SoftwareResetDriver::reset(void)
{
  esp_restart();
}
