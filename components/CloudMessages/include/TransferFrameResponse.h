#pragma once

#include <cstdint>

#include "CloudOutboundMessageInterface.h"
#include "JSONObject.h"


class TransferFrameResponse : public CloudOutboundMessageInterface
{
public:
  TransferFrameResponse(void);
  TransferFrameResponse(const char * base64Frame);

  void setFrame(const char * base64Frame);
  void makeEmptyJson();

  virtual char * getJSONString(void) const;

private:
  JSONObject m_message;
};
