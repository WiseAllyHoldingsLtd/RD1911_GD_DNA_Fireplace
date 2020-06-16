#include "TransferFrameRequest.h"


namespace
{
  const char TIMEOUT_KEY[] = "timeoutInMilliSec";
  const char FRAME_KEY[] = "frame";
}

TransferFrameRequest::TransferFrameRequest(void)
{
}


bool TransferFrameRequest::parseJSONString(const char * jsonString)
{
  bool parseResult = m_message.parse(jsonString);

  if (parseResult)
  {
    int32_t dummyValue;
    parseResult = (getTimeoutMSValue(dummyValue) && (getFrame() != nullptr));
  }

  return parseResult;
}


int32_t TransferFrameRequest::getTimeoutMS(void) const
{
  int32_t returnValue = 0;
  getTimeoutMSValue(returnValue);
  return returnValue;
}


const char * TransferFrameRequest::getFrame(void) const
{
  return m_message.getString(FRAME_KEY);
}


bool TransferFrameRequest::getTimeoutMSValue(int32_t & value) const
{
  return m_message.getValue(TIMEOUT_KEY, value);
}
