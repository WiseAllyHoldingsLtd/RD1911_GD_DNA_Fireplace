#pragma once

#include "IHTTPData.h"

/** A data endpoint to store text
*/
class HTTPBinary : public IHTTPDataIn
{
public:
  HTTPBinary(char * data, size_t size);

  virtual size_t getContentLength(void) const;
  virtual size_t getDataLength(void) const;

protected:
  /* IHTTPDataIn */
  virtual void writeReset(void);
  virtual int write(const char* buf, size_t len);
  virtual void setDataType(const char* type);
  virtual void setIsChunked(bool chunked);
  virtual void setDataLen(size_t len);

private:
  char * m_data;
  size_t m_size;
  size_t m_pos;
  bool m_isChunked;
  size_t m_contentLength;
};
