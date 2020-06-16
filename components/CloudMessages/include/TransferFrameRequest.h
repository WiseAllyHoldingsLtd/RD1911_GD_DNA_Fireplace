#pragma once

#include <cstdint>

#include "CloudInboundMessageInterface.h"
#include "JSONObject.h"


class TransferFrameRequest : public CloudInboundMessageInterface
{
public:
  TransferFrameRequest(void);
  virtual bool parseJSONString(const char * jsonString);

  int32_t getTimeoutMS(void) const;
  const char * getFrame(void) const;

private:
  bool getTimeoutMSValue(int32_t & value) const;
  JSONObject m_message;
};
