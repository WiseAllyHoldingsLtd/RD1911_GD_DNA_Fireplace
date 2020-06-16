#include "TransferFrameResponse.h"


namespace
{
  const char FRAME_KEY[] = "frame";
}

TransferFrameResponse::TransferFrameResponse(void)
{
  setFrame(nullptr);
}


TransferFrameResponse::TransferFrameResponse(const char * base64Frame)
{
  setFrame(base64Frame);
}

void TransferFrameResponse::setFrame(const char * base64Frame)
{
  m_message.create();

  if (base64Frame != nullptr)
  {
    m_message.addString(FRAME_KEY, base64Frame);
  }
  else
  {
    m_message.addNull(FRAME_KEY);
  }
}

void TransferFrameResponse::makeEmptyJson()
{
  m_message.create(); // will re-create empty object if object already exists
}

char * TransferFrameResponse::getJSONString(void) const
{
  return m_message.printJSON();
}
