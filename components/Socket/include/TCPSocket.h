#pragma once

#include <cstdint>

#include "lwip/api.h"

#include "SocketInterface.h"


class TCPSocket : public SocketInterface
{
public:
  TCPSocket();
  virtual ~TCPSocket();

  virtual bool connect(const char* host, const int port);
  virtual bool isConnected(void) const;
  virtual int send(const char* data, int length);
  virtual int sendAll(const char* data, int length);
  virtual int receive(char* data, int length);
  virtual int receiveAll(char* data, int length);
  virtual void setBlocking(bool blocking, unsigned int timeout=1500);
  virtual void close();

private:
  struct netconn *m_connection;
  bool m_isConnected;
  struct netbuf *m_receiveBuffer;
  uint16_t m_bufferBytes;
  uint16_t m_bufferPosition;
};

