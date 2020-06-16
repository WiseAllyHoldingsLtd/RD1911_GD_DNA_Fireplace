#pragma once

#include <cstdint>

#include "CloudOutboundMessageInterface.h"
#include "JSONObject.h"


class DiscoveryRequest : public CloudOutboundMessageInterface
{
public:
  DiscoveryRequest(const char * gdid, uint8_t swVersion, uint8_t testVersion);
  virtual char * getJSONString(void) const;

private:
  JSONObject m_message;
};
