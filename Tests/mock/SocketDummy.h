#pragma once

#include "stdint.h";
#include "assert.h"
#include <cstring>
#include "SocketInterface.h"


class SocketDummy : public SocketInterface
{
public:
  SocketDummy(void)
    : m_sendBufferSize(0U), m_receiveBufferSize(0U), m_readPos(0U)
  {
  }

  virtual bool connect(const char* host, const int port)
  {
    return true;
  }

  virtual bool isConnected(void) const
  {
    return true;
  }

  virtual int send(const char* data, int length)
  {
    int sentBytes = 0;

    if ((m_sendBufferSize + length) <= BUFFER_SIZE)
    {
      memcpy(&m_sendBuffer[m_sendBufferSize], data, length);
      m_sendBufferSize += length;
      sentBytes = length;
    }

    return sentBytes;
  }

  virtual int sendAll(const char* data, int length)
  {
    return send(data, length);
  }

  virtual int receive(char* data, int length)
  {
    int readBytes = 0U;

    uint32_t dataAvailable = 0U;

    if ((m_readPos < m_receiveBufferSize))
    {
      dataAvailable = m_receiveBufferSize - m_readPos;
    }

    if (dataAvailable > 0U)
    {
      if (dataAvailable < length)
      {
        memcpy(data, &m_receiveBuffer[m_readPos], dataAvailable);
        m_readPos += dataAvailable;
        readBytes = dataAvailable;
      }
      else
      {
        memcpy(data, &m_receiveBuffer[m_readPos], length);
        m_readPos += length;
        readBytes = length;
      }
    }

    return readBytes;
  }

  virtual int receiveAll(char* data, int length)
  {
    return receive(data, length);
  }

  virtual void setBlocking(bool blocking, unsigned int timeout=1500)
  {
  }

  virtual void close()
  {
  }


  /* Test helpers */

  void setReceiveBuffer(const char* data, uint32_t size)
  {
    assert(size <= BUFFER_SIZE);

    memcpy(m_receiveBuffer, data, size);
    m_receiveBufferSize = size;
    m_readPos = 0U;
  }

  bool sendBufferCompare(const char * buffer, uint32_t size) const
  {
    bool isSizeEqual = (size == m_sendBufferSize);
    bool isContentEqual = false;

    if (isSizeEqual)
    {
      isContentEqual = (memcmp(buffer, m_sendBuffer, m_sendBufferSize) == 0);
    }

    return (isSizeEqual && isContentEqual);
  }

private:
  static const uint32_t BUFFER_SIZE = 4096U;
  char m_sendBuffer[BUFFER_SIZE];
  uint32_t m_sendBufferSize;

  char m_receiveBuffer[BUFFER_SIZE];
  uint32_t m_receiveBufferSize;
  uint32_t m_readPos;
};
