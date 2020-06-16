#include "HTTPHead.h"

#define OK 0


HTTPHead::HTTPHead(void)
  : m_isChunked(false), m_contentLength(0)
{
}


size_t HTTPHead::getContentLength(void) const
{
  return m_contentLength;
}


size_t HTTPHead::getDataLength(void) const
{
  return 0U;
}


/* IHTTPDataIn */

void HTTPHead::writeReset(void)
{
  /* Not used */
}


int HTTPHead::write(const char* buf, size_t len)
{
  return OK;
}


void HTTPHead::setDataType(const char* type)
{
  /* Not used */
}


void HTTPHead::setIsChunked(bool chunked)
{
  m_isChunked = chunked;
}


void HTTPHead::setDataLen(size_t len)
{
  if (m_isChunked)
  {
    m_contentLength += len;
  }
  else
  {
    m_contentLength = len;
  }
}
