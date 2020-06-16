#include "esp_log.h"

#include "TCPSocket.h"

static const char * LOG_TAG = "TCPSocket";


TCPSocket::TCPSocket()
: m_connection(nullptr), m_isConnected(false), m_receiveBuffer(nullptr), m_bufferBytes(0u), m_bufferPosition(0u)
{
}

TCPSocket::~TCPSocket()
{
  close();
}


bool TCPSocket::connect(const char* host, const int port)
{
  bool result = false;

  // Create connection
  m_connection = netconn_new(NETCONN_TCP);
  if (m_connection != nullptr) {
    // Resolve host
    ip_addr_t address = {};
    if (netconn_gethostbyname(host, &address) == ERR_OK) {
      // connect
      if (netconn_connect(m_connection, &address, static_cast<uint16_t>(port)) == ERR_OK) {
        result = true;
        m_isConnected = true;
      }
      else {
        ESP_LOGE(LOG_TAG, "Failed to connect");
      }
    }
    else {
      ESP_LOGE(LOG_TAG, "Failed to look up host");
    }
  }
  else {
    ESP_LOGE(LOG_TAG, "Failed at creating connection");
  }

  return result;
}

bool TCPSocket::isConnected(void) const
{
  return m_isConnected;
}

int TCPSocket::send(const char* data, int length)
{
  int result = -1;

  size_t bytesWritten = 0;
  if (netconn_write_partly(m_connection, static_cast<const void*>(data), length, NETCONN_NOCOPY, &bytesWritten) == ERR_OK) {
    result = static_cast<int>(bytesWritten);
    // FIXME: send all?
  }
  else {
    ESP_LOGE(LOG_TAG, "Failed to send data");
  }

  return result;
}

int TCPSocket::sendAll(const char* data, int length)
{
  int bytesSent = 0;
  while (bytesSent < length) {
    int bytesLeft = length - bytesSent;
    int retval = send(&data[bytesSent], bytesLeft);
    if (retval > 0) {
      bytesSent += retval;
    }
    else {
      ESP_LOGE(LOG_TAG, "Failed to send all: %i", retval);
      bytesSent = retval;
      break;
    }
  }

  return bytesSent;
}

int TCPSocket::receive(char* data, int length)
{
  int result = -1;

  // Receive new data if no current m_receiveBuffer
  if (m_receiveBuffer == nullptr) {
    err_t res = netconn_recv(m_connection, &m_receiveBuffer);
    if (res == ERR_OK) {
      m_bufferBytes = netbuf_len(m_receiveBuffer);
      m_bufferPosition = 0u;
      ESP_LOGD(LOG_TAG, "received %u bytes, caller has room for %i", m_bufferBytes, length);
    }
    else if (res == ERR_CLSD) {
      // Connection was closed by server
      ESP_LOGI(LOG_TAG, "Remote server closed connection");
      m_isConnected = false;
      result = 0; // zero bytes
    }
    else if (res == ERR_TIMEOUT) {
      ESP_LOGD(LOG_TAG, "Receive timed out");
      result = 0; // zero bytes
    }
    else {
      ESP_LOGE(LOG_TAG, "Failed to receive data: %i", static_cast<int>(res));
    }
  }

  // Copy data from m_receiveBuffer to caller
  if (m_receiveBuffer != nullptr) {
    uint16_t bytesReceieved = netbuf_copy_partial(m_receiveBuffer, static_cast<void*>(data), length, m_bufferPosition);
    result = static_cast<int>(bytesReceieved);
    if (bytesReceieved == m_bufferBytes) {
      // m_receiveBuffer empty, reset for new reading
      netbuf_delete(m_receiveBuffer);
      m_receiveBuffer = nullptr;
      m_bufferBytes = 0u;
      m_bufferPosition = 0u;
    }
    else {
      // remaining data. update counters for next call
      m_bufferBytes -= bytesReceieved;
      m_bufferPosition += bytesReceieved;
    }
  }
  return result;
}

int TCPSocket::receiveAll(char* data, int length)
{
  // FIXME: Tamper around with timeouts to test if there is more data?
  return receive(data, length);
}

void TCPSocket::setBlocking(bool blocking, unsigned int timeout)
{
  netconn_set_nonblocking(m_connection, blocking);
  int newTimeout = static_cast<int>(timeout);
  if (newTimeout == 0) {  // setting timeout to 0 causes blocking forever.
    newTimeout = 1;
  }
  netconn_set_recvtimeout(m_connection, newTimeout);
}

void TCPSocket::close()
{
  if (m_receiveBuffer != nullptr) {
    netbuf_delete(m_receiveBuffer);
    m_receiveBuffer = nullptr;
    m_bufferBytes = 0u;
    m_bufferPosition = 0u;
  }
  if (m_isConnected) {
    err_t res = netconn_close(m_connection);
    if (res != ERR_OK) {
      ESP_LOGE(LOG_TAG, "Failed to close: %i", static_cast<int>(res));
    }
    m_isConnected = false;
  }
  if (m_connection != nullptr) {
    if (netconn_delete(m_connection) != ERR_OK) {
      ESP_LOGE(LOG_TAG, "Failed to delete connection");
    }
    m_connection = nullptr;
  }
}
