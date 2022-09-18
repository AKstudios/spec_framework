//==========================================================================================================
// netsock.h - Defines a network socket
//
// Presume that most of the functions in this class can throw runtime_error exceptions.
//==========================================================================================================
#pragma once
#include <netinet/in.h>
#include <string>
#include <stdexcept>

class NetSock
{
public:

    // The are the codes that can be returned by get_error()
    enum
    {
        BIND_FAILED,
        NO_SUCH_SERVER,
        CANT_CONNECT
    };


    // Constructor and Destructor
    NetSock();
    ~NetSock() {close();}

    // Copy constructor
    NetSock(const NetSock& rhs) {copy_object(rhs);}
    
    // Assignment 
    NetSock& operator=(const NetSock& rhs) {copy_object(rhs); return *this;}

    // Call this to create a server socket
    bool    create_server(int port, std::string bind_to = "", int family = AF_UNSPEC);

    // Call this to start listening for connections.  Can throw runtime_error
    void    listen(int concurrent_connections = 0);

    // Call this to wait for someone to connect to a server socket
    bool    accept(int timeout_ms = -1, NetSock* new_sock = NULL);

    // Convenience call for waiting for a single incoming connection
    bool    listen_and_accept(int timeout_ms = -1);

    // Call this to connect to a server
    bool    connect(std::string server_name, int port, int family = AF_INET);

    // Call this to turn Nagle's algorithm on or off
    void    set_nagling(bool flag);

    // After an "accept()", call this to find the IP address of the client
    std::string get_peer_address();

    // Waits for data to arrive.  Returns 'true' if data became available before the timeout expires
    bool    wait_for_data(int milliseconds);

    // Returns the number of bytes available for reading 
    int     bytes_available();

    // Call this to receive a fixed amount of data from the socket
    int     receive(void* buffer, int length, bool peek = false);

    // Call this to receive however many bytes are available for reading. Returns the
    // the number of bytes received, or -1 if the socket was closed by the other side.
    int     receive_noblock(void* buffer, int length);

    // Waits for data to arrive, the receives however much is available
    int     receive_fragment(void *buffer, size_t buffsize);

    // Call this to fetch a line of text from the socket
    bool    getline(void* buffer, size_t buff_size);

    // Call these to send a string or buffer full of data
    int     send(std::string s);
    int     send(const void* buffer, int length);

    // Call this to send data using print-style formatting
    int     sendf(const char* fmt, ...);

    // Call this to close this socket.  Safe to call if socket isn't open
    void    close();

    // When connect(), create() (etc) fail, this will give information about the error
    int     get_error(std::string* p_str = NULL);

protected:

    // Copy another object of this type
    void    copy_object(const NetSock& rhs);

    // Most recent error
    std::string m_error_str;
    int     m_error;

    // This will be true on a socket for which create_server() or connect() has been called
    bool    m_is_created;

    // The socket descriptor of our socket
    int     m_sd;
};