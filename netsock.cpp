//==========================================================================================================
// netsock.cpp - Implements a network socket
//==========================================================================================================
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include "netsock.h"
#include "netutil.h"
using namespace std;



//==========================================================================================================
// Constructor
//==========================================================================================================
NetSock::NetSock()
{
    // Ensure that we start out with an invalid socket descriptor
    m_sd = -1;

    // This socket has not yet been created
    m_is_created = false;
}
//==========================================================================================================


//==========================================================================================================
// copy_object() - Copies another object of the same type
//==========================================================================================================
void NetSock::copy_object(const NetSock& rhs)
{
    m_sd         = rhs.m_sd;
    m_is_created = rhs.m_is_created;
    m_error      = rhs.m_error;
    m_error_str  = rhs.m_error_str;
}
//==========================================================================================================


//==========================================================================================================
// close() - Closes the socket
//==========================================================================================================
void NetSock::close()
{
    if (m_sd >= 0) ::close(m_sd);
    m_sd = -1;
}
//==========================================================================================================


//==========================================================================================================
// connect() - Creates the socket and connects it to a server
//==========================================================================================================
bool NetSock::connect(std::string server, int port, int family)
{
    addrinfo info;

    // The socket is not yet created
    m_is_created = false;

    // Close this socket if it happens to be open
    close();

    // If we can't find any information about that server, tell the caller
    if (!NetUtil::get_server_addrinfo(SOCK_STREAM, server, port, family, &info)) 
    {
        m_error_str = "no such server: "+server;
        m_error     = NO_SUCH_SERVER;
        return false;
    }

    // Create the socket
    m_sd = socket(info.ai_family, info.ai_socktype, info.ai_protocol);

    // If the socket() call fails, complain
    if (m_sd < 0) throw runtime_error("failure on socket()");

    // Attempt to connect to the server
    if (::connect(m_sd, info.ai_addr, info.ai_addrlen) < 0)
    {
        m_error_str = "can't connect to "+server;
        m_error     = CANT_CONNECT;
        close();
        return false;
    }

    // If we get here, we have a connected socket
    return true;
}
//==========================================================================================================



//==========================================================================================================
// create_server() - Creates a server socket
//
// Passed:  port    = The TCP port number to create the socket on
//          bind_to = The IP address of the network card to bind to (optional)
//          family  = AF_UNSPEC, AF_INET, or AF_INET6
//
// Returns: 'true'  = The server socket was created succesfully.
//          'false' = The call to "bind" failed, typically because another socket is already
//                    bound to that port.
//==========================================================================================================
bool NetSock::create_server(int port, string bind_to, int family)
{
    // The socket is not yet created
    m_is_created = false;

    // Close this socket if it happens to be open
    close();

    // Fetch information about the server socket we're going to create
    addrinfo server = NetUtil::get_local_addrinfo(SOCK_STREAM, port, bind_to, family);

    // If that call to "get_local_info" failed, we throw an exception
    if (server.ai_family == 0) throw runtime_error("failure on getaddrinfo()");

    // Create the socket
    m_sd = socket(server.ai_family, server.ai_socktype, server.ai_protocol);

    // If the socket() call fails, complain
    if (m_sd < 0) throw runtime_error("failure on socket()");

    // Allow us to re-use this port if it's still in TIME_WAIT
    int optval = 1;
    setsockopt(m_sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    // Bind it to the port we passed in to getaddrinfo():
    if (bind(m_sd, server.ai_addr, server.ai_addrlen) < 0)
    {
        m_error_str = "failure on bind()";
        m_error     = BIND_FAILED;
        close();
        return false;
    }

    // This socket has been created
    m_is_created = true;

    // Tell the caller that all is well
    return true;
}
//==========================================================================================================


//==========================================================================================================
// listen() - Starts listening for connections on a server socket.   
//
// Throws runtime_error if something goes wrong.
//==========================================================================================================
void NetSock::listen(int concurrent_connections)
{
    // If the socket isn't created yet, don't even try 
    if (!m_is_created) throw runtime_error("listen called on non existent socket");

    // Start listening for an incoming connection
    int status = ::listen(m_sd, concurrent_connections);

    // If listen() barfed on us, tell the caller
    if (status < 0) throw runtime_error("failure on listen()");
}
//==========================================================================================================


//==========================================================================================================
// accept() - Waits for someone to connect to a server socket
// 
// Passed: timeout_ms = # of milliseconds to wait for incoming connection.  -1 = Wait forever
//         dest_sock  = The optional address of a destination NetSock object to attach the connection to
//
// Returns: true if a client connected to our server, otherwise false
//==========================================================================================================
bool NetSock::accept(int timeout_ms, NetSock* dest_sock)
{
    // If the socket isn't created yet, don't even try 
    if (!m_is_created) throw runtime_error("accept called on non existent socket");

    // Wait for a client to connect to our socket
    if (!wait_for_data(timeout_ms)) return false;

    // Accept the connection
    int new_sd = ::accept(m_sd, NULL, 0);

    // If accept() failed, tell the caller
    if (new_sd < 0) throw runtime_error("failure on accept()");

    // If the caller passed us a socket object to clone ourselves into...
    if (dest_sock)
    {
        *dest_sock = *this;
        dest_sock->m_sd = new_sd;
    }

    // Otherwise, the new socket-descriptor is the one we'll read and write on
    else
    {
        ::close(m_sd);
        m_sd = new_sd;        
    }

    // Tell the caller that all is well
    return true;
}
//==========================================================================================================



//==========================================================================================================
// listen_and_accept() - Convience method for waiting for a single incoming connection
// 
// Passed: timeout_ms = # of milliseconds to wait for incoming connection.  -1 = Wait forever
//
// Returns: true if a client connected to our server, otherwise false
//==========================================================================================================
bool NetSock::listen_and_accept(int timeout_ms)
{
    listen(0);
    return accept(timeout_ms);
}
//==========================================================================================================



//==========================================================================================================
// get_peer_address() - Returns the IP address of the other side of the connection
//==========================================================================================================
string NetSock::get_peer_address()
{
    sockaddr_storage peer_addr;

    // getpeername() will need to know the size of peer_addr
    socklen_t addr_size = sizeof(peer_addr);

    // Get a convenient pointer to the peer address
    sockaddr* p_peer = (sockaddr*)&peer_addr;

    // Fetch the IP address of the machine on the other side of the socket
    if (getpeername(m_sd, p_peer, &addr_size) < 0) return "unknown";

    // Hand the caller the IP address    
    return NetUtil::ip_to_string(p_peer);
}
//==========================================================================================================


//==========================================================================================================
// set_nagling() - Turn Nagle's algorithm on or off for this socket.
//
// When Nagle's algorithm is on, the socket will attempt to coalesce data before sending it to the network.
// When Nagle's algorithm is off, all writes will be sent to the network immediately
//==========================================================================================================
void NetSock::set_nagling(bool flag)
{
    setsockopt(m_sd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof flag);
}
//==========================================================================================================


//==========================================================================================================
// wait_for_data() - Waits for the specified amount of time for data to be available for reading
//
// Passed: timeout_ms = timeout in milliseconds.  -1 = Wait forever
//
// Returns: true if data is available for reading, else false
//==========================================================================================================
bool NetSock::wait_for_data(int timeout_ms)
{
    return NetUtil::wait_for_data(timeout_ms, m_sd);
}
//==========================================================================================================


//==========================================================================================================
// bytes_available() - Returns the number of bytes available for reading
//==========================================================================================================
int NetSock::bytes_available()
{
    int count = 0;
    ioctl(m_sd, FIONREAD, &count);
    return count;
}
//==========================================================================================================



//==========================================================================================================
// receive() - Receives data from the socket
//
// Passed:  buffer = Pointer to the place to store the received data
//          length = The number of bytes to read in
//          peek   = If true, the bytes will be returned but not removed from the buffer
//
// Returns: The number of bytes that were read
//             -- or -- -1 = An error occured
//             -- or --  0 = The socket was closed (possibly by the other side)
//==========================================================================================================
int NetSock::receive(void* buffer, int length, bool peek)
{
    // Don't attempt to recv zero bytes
    if (length == 0) return 0;

    // This is the set of flags that we're going to pass to recv
    int flags = peek ?  MSG_PEEK : 0;

    // Get a byte-pointer to the caller's buffer
    unsigned char* ptr = (unsigned char*)buffer;

    // Keep track of how many bytes we have left to read
    int bytes_remaining = length;

    // Loop until there are no more bytes to read...
    while (bytes_remaining)
    {
        // Fetch some bytes from the socket
        int bytes_rcvd = recv(m_sd, ptr, bytes_remaining, flags);

        // If the read failed, tell the caller
        if (bytes_rcvd < 0) return -1;

        // If the socket is closed, tell the caller
        if (bytes_rcvd == 0) return 0;

        // Adjust our pointer and the number of bytes remaining to be read
        ptr             += bytes_rcvd;
        bytes_remaining -= bytes_rcvd;
    }

    // Tell the caller that we received all of the data they wanted
    return length;
}
//==========================================================================================================




//==========================================================================================================
// receive_fragment() - Receives data from the socket
//
// Passed:  buffer = Pointer to the place to store the received data
//          bufsize= The length of the buffer in bytes
//
// Returns: The number of bytes that were read
//             -- or -- -1 to indicate the socket was closed by the peer
//==========================================================================================================
int NetSock::receive_fragment(void* buffer, size_t bufsize)
{

    // Fetch some bytes from the socket
    int bytes_rcvd = recv(m_sd, buffer, bufsize, 0);

    // Return either the number of bytes received, or -1 to signal "socket was closed"
    return (bytes_rcvd < 1) ? -1 : bytes_rcvd;
}
//==========================================================================================================



//==========================================================================================================
// receive_noblock() - Receives data from the socket, with a gaurantee to not block
//
// Passed:  buffer = Pointer to the place to store the received data
//          length = The maximum number of bytes to receive
//
// Returns: The number of bytes that were read
//             -- or -- -1 = The socket was closed by the peer
//==========================================================================================================
int NetSock::receive_noblock(void* buffer, int length)
{
    // Don't attempt to recv zero bytes
    if (length == 0) return 0;

    // Fetch as many bytes from the socket as we can
    int bytes_rcvd = recv(m_sd, buffer, length, MSG_DONTWAIT);

    // If we got an error indicator...
    if (bytes_rcvd == -1)
    {
        // If the error was "no data available", tell the caller "no bytes received"
        if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
        
        // Otherwise, a true error occured (i.e., the peer closed the socket)
        close();
        return -1;
    }

    // Tell the caller how many bytes were received
    return bytes_rcvd;
}
//==========================================================================================================




//==========================================================================================================
// getline() - Fetches a line of text from the socket
//
// The result buffer doesn't include the terminating carriage-return/linefeed
//==========================================================================================================
bool NetSock::getline(void* buffer, size_t buff_size)
{
    char c, *ptr, *origin;

    // Don't let the caller pass us a buffer size of zero
    if (buff_size == 0) return false;

    // Reduce the buffer size by 1 to allow for appending the nul-byte to the end of it
    --buff_size;

    // Get a byte pointer to the caller's buffer
    origin = ptr = (char*) buffer;

    // Loop until either an error or until we see a linefeed
    while (true)
    {
        // Fetch a single byte from the socket
        if (recv(m_sd, &c, 1, 0) < 1) return false;

        // If it's a carriage-return, throw it away
        if (c == '\r') continue;

        // Handle backspace, in case the client is a human-being typing
        if (c == 8)
        {
            if (ptr > origin) --ptr;
            continue;            
        }

        // If it's a line-feed, it's the end of the line
        if (c == '\n') break;

        // If this character will fit into the caller's buffer, append it there
        if ((ptr - origin) < buff_size) *ptr++ = c;
    }

    // We've encountered the end of the line.  Terminate the output string
    *ptr = 0;

    // And tell the caller that he has a line of data waiting in his buffer
    return true;
}
//==========================================================================================================


//==========================================================================================================
// send() - Sends a string to the other side of a connected socket
//
// Returns either : -1 = An error occured
//                  Anything else = the number of bytes actually sent.  The entire string will always be
//                  sent unless the socket was closed by the other side
//==========================================================================================================
int NetSock::send(string s)
{
    return send(s.c_str(), s.size());
}
//==========================================================================================================


//==========================================================================================================
// send() - Sends a buffer to the other side of a connected socket
//
// Returns either : -1 = An error occured
//                  Anything else = the number of bytes actually sent.  The entire string will always be
//                  sent unless the socket was closed by the other side
//==========================================================================================================
int NetSock::send(const void* buffer, int length)
{
    // Don't attempt to send zero bytes
    if (length == 0) return 0;

    // Get a byte pointer to the caller's buffer
    unsigned char* ptr = (unsigned char*)buffer;

    // Keep track of how many bytes remain to be sent
    int bytes_remaining = length;

    // Loop until there are no more bytes to send...
    while (bytes_remaining)
    {
        // Attempt to send all of the bytes
        int sent = ::send(m_sd, ptr, bytes_remaining, MSG_NOSIGNAL);

        // If an error occured, tell the caller
        if (sent < 0) return -1;

        // If the socket is closed, we're done
        if (sent == 0) break;

        // Adjust the pointer and the count of bytes remaining to be sent
        ptr             += sent;
        bytes_remaining -= sent;
    }

    // Tell the caller how many bytes we sent
    return (length - bytes_remaining);
}
//==========================================================================================================



//==========================================================================================================
// sendf() - Sends a printf-style formatt data to the the other side of a connected socket
//
// Returns either : -1 = An error occured
//                  Anything else = the number of bytes actually sent.  The entire string will always be
//                  sent unless the socket was closed by the other side
//==========================================================================================================
int NetSock::sendf(const char* fmt, ...)
{
    char buffer[1000];

    // This is a pointer to the variable argument list
    va_list ap;

    // Point to the first argument after the "fmt" parameter
    va_start(ap, fmt);

    // Perform a printf of our arguments into a buffer area;
    vsnprintf(buffer, sizeof buffer, fmt, ap);

    // Tell the system that we're done with the "ap"
    va_end(ap);

    // And send the buffer
    return send(buffer, strlen(buffer));
}
//==========================================================================================================



//==========================================================================================================
// get_error() - Returns information about the most recent failure
//==========================================================================================================
int NetSock::get_error(string* p_str)
{
    if (p_str) *p_str = m_error_str;
    return m_error;
}
//==========================================================================================================

