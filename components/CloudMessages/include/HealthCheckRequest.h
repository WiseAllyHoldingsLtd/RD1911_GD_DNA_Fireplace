#pragma once

#include <cstdint>

#include "CloudOutboundMessageInterface.h"
#include "JSONObject.h"


class HealthCheckRequest : public CloudOutboundMessageInterface
{
public:
  HealthCheckRequest(const char * gdid);
  virtual char * getJSONString(void) const;

private:
  JSONObject m_message;
};
