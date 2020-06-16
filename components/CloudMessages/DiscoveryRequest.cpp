#include "DiscoveryRequest.h"

#include <cstdio>
#include "StaticStrings.h"


DiscoveryRequest::DiscoveryRequest(const char * gdid, uint8_t swVersion, uint8_t testVersion)
{
  char versionStr[12U] = {}; /* Ex: 01.03.02 */
  snprintf(versionStr,
           sizeof(versionStr) - 1U,
           StaticStrings::CLOUD_DEVICE_VERSION_FORMAT,
           (swVersion / 10u),
           (swVersion % 10u),
           testVersion);

  m_message.create();
  m_message.addString("Gdid", gdid);
  m_message.addString("Version", versionStr);
  m_message.addBool("DirectAppliance", true); // always true for Wifi appliance
}

char * DiscoveryRequest::getJSONString(void) const
{
  return m_message.printJSON();
}
