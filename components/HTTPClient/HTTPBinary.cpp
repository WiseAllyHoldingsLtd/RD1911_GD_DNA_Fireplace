#include <cstring>
#include "HTTPBinary.h"

#define OK 0

using std::memcpy;
using std::strncpy;
using std::strlen;

#define MIN(x,y) (((x)<(y))?(x):(y))


HTTPBinary::HTTPBinary(char * data, size_t size)
  : m_data(data), m_size(size), m_pos(0), m_isChunked(false), m_contentLength(0)
{
}


size_t HTTPBinary::getContentLength(void) const
{
  return m_contentLength;
}


size_t HTTPBinary::getDataLength(void) const
{
  return m_pos;
}


/* IHTTPDataIn */

void HTTPBinary::writeReset(void)
{
  m_pos = 0;
}


int HTTPBinary::write(const char* buf, size_t len)
{
  size_t writeLen = MIN(len, m_size - m_pos);
  memcpy(m_data + m_pos, buf, writeLen);
  m_pos += writeLen;
  return OK;
}


void HTTPBinary::setDataType(const char* type)
{
  /* Not used */
}


void HTTPBinary::setIsChunked(bool chunked)
{
  m_isChunked = chunked;
}


void HTTPBinary::setDataLen(size_t len)
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
