#pragma once

#include "IHTTPData.h"

/** A data endpoint to store text
*/
class HTTPHead : public IHTTPDataIn
{
public:
  HTTPHead(void);

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
  bool m_isChunked;
  size_t m_contentLength;
};
