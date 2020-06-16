#pragma once

#include <cstdint>

#include "CloudInboundMessageInterface.h"
#include "JSONObject.h"


class DiscoveryResponse : public CloudInboundMessageInterface
{
public:
  DiscoveryResponse(void);
  virtual bool parseJSONString(const char * jsonString);

  const char * getPrimaryConnectionString(void) const;
  const char * getSecondaryConnectionString(void) const;

private:
  JSONObject m_message;
};
