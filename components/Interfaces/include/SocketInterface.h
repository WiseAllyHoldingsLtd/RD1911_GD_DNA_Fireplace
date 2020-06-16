#pragma once


class SocketInterface {
public:
    virtual ~SocketInterface() {}

    /** Connects this TCP socket to the server
    \param host The host to connect to. It can either be an IP Address or a hostname that will be resolved with DNS.
    \param port The host's port to connect to.
    \return true on success, false on failure.
    */
    virtual bool connect(const char* host, const int port) = 0;

    /** Check if the socket is connected
    \return true if connected, false otherwise.
    */
    virtual bool isConnected(void) const = 0;

    /** Send data to the remote host.
    \param data The buffer to send to the host.
    \param length The length of the buffer to send.
    \return the number of written bytes on success (>=0) or -1 on failure
     */
    virtual int send(const char* data, int length) = 0;

    /** Send all the data to the remote host.
    \param data The buffer to send to the host.
    \param length The length of the buffer to send.
    \return the number of written bytes on success (>=0) or -1 on failure
    */
    virtual int sendAll(const char* data, int length) = 0;

    /** Receive data from the remote host.
    \param data The buffer in which to store the data received from the host.
    \param length The maximum length of the buffer.
    \return the number of received bytes on success (>=0) or -1 on failure
     */
    virtual int receive(char* data, int length) = 0;

    /** Receive all the data from the remote host.
    \param data The buffer in which to store the data received from the host.
    \param length The maximum length of the buffer.
    \return the number of received bytes on success (>=0) or -1 on failure
    */
    virtual int receiveAll(char* data, int length) = 0;


    /** Set blocking or non-blocking mode of the socket and a timeout on
        blocking socket operations
    \param blocking  true for blocking mode, false for non-blocking mode.
    \param timeout   timeout in ms [Default: (1500)ms].
    */
    virtual void setBlocking(bool blocking, unsigned int timeout=1500) = 0;

    /** Close the socket
     */
    virtual void close() = 0;
};

