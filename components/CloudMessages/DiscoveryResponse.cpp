#include "DiscoveryResponse.h"


DiscoveryResponse::DiscoveryResponse(void)
{
}

bool DiscoveryResponse::parseJSONString(const char * jsonString)
{
  bool parseResult = m_message.parse(jsonString);

  if (parseResult)
  {
    parseResult = ((getPrimaryConnectionString() != nullptr) && (getSecondaryConnectionString() != nullptr));
  }

  return parseResult;
}


const char * DiscoveryResponse::getPrimaryConnectionString(void) const
{
  return m_message.getString("Primary");
}


const char * DiscoveryResponse::getSecondaryConnectionString(void) const
{
  return m_message.getString("Secondary");
}
