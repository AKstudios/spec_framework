//============================================================================
// Class CSerialPort - Implements an API for raw serial I/O services
//============================================================================
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdarg.h>
#include "serial_port.h"
using std::string;

// We're going to use this as a place to dump unused return values
static volatile int bitbucket;

//============================================================================
// Constructor() - Serial port begins in the 'closed' state
//============================================================================
CSerialPort::CSerialPort() {m_fd = -1; m_default_timeout_ms = SP_NO_TIMEOUT;}
//============================================================================


//============================================================================
// Destructor() - Closes the serial port if it's open
//============================================================================
CSerialPort::~CSerialPort() {close();}
//============================================================================


//============================================================================
// set_default_read_timeout() - Sets the default timeout for all operations 
//                              that read data from the serial port.   This 
//                              includes read(), get_line() and get_char()
//============================================================================
void CSerialPort::set_default_read_timeout(int milliseconds)
{
    m_default_timeout_ms = milliseconds;
}
//============================================================================


//============================================================================
// close() - Closes the serial port if it's open
//============================================================================
void CSerialPort::close()
{
    // If the serial port is open, close it
    if (m_fd >= 0) ::close(m_fd);

    // Indicate that there is no serial port open
    m_fd = -1;
}
//============================================================================


//=============================================================================
// baud_to_constant() - Translates an integer baud rate to one of the speed_t
//                      constants
//
// Returns:  One of the baud-rate constants from termios.h
//              -- OR --
//           (speed_t)0 to indicate the requested baud-rate is insupported
//=============================================================================
speed_t CSerialPort::baud_to_constant(uint32_t baud)
{
    switch (baud)
    {
        case 300:       return B300;
        case 1200:      return B1200;
        case 9600:      return B9600;
        case 19200:     return B19200;
        case 38400:     return B38400;
        case 57600:     return B57600;
        case 115200:    return B115200;
    };

    // Tell the caller that we don't support the baud-rate
    return (speed_t)0;
}
//=============================================================================


//============================================================================
// open() - Opens a connection to the serial port.
//============================================================================
bool CSerialPort::open(string device, uint32_t baud)
{
    termios tio;

    // Make sure that any currently open connection is closed
    close();

    // Convert the integer baud-rate into one of the termios speed constants
    speed_t speed = baud_to_constant(baud);

    // If the requested baud-rate was illegal, tell the caller
    if (speed == (speed_t)0) return false;

    // Open the device file
    m_fd = ::open(device.c_str(), O_RDWR | O_NOCTTY);

    // If we can't open the device, tell the caller
    if (m_fd < 0) return false;

    // Start out with a blank "termios" structure
    memset(&tio, 0, sizeof(tio));

    // Fill in the settings that make this a non-canonical (i.e., raw) port
    cfmakeraw(&tio);

    // Set up the speed (and 8/N/1)
    tio.c_cflag = speed | CS8 | CLOCAL | CREAD;

    // Set the settings for this serial port
    tcsetattr(m_fd, TCSANOW, &tio);

    // Tell the caller that all is well
    return true;
}
//============================================================================


//============================================================================
// data_is_available() - Waits for data to become available for reading on the
//                     serial port.
//
// Passed:  The number of milliseconds to wait for data to become available.
//          If this is SP_NO_TIMEOUT, we will wait forever.
//          If this is SP_DEFAULT_TIMEOUT, we will use the default timeout
//
// Returns: 'true' if data is available for reading, otherwise 'false'
//============================================================================
bool CSerialPort::data_is_available(int timeout_ms)
{
    fd_set  rfds;
    timeval timeout;

    // If we're supposed to use the default timeout, do so
    if (timeout_ms == SP_DEFAULT_TIMEOUT) timeout_ms = m_default_timeout_ms;

    // Assume for the moment that we are going to wait forever
    timeval* pTimeout = NULL;

    // If the caller wants us to wait for a finite amount of time...
    if (timeout_ms != SP_NO_TIMEOUT)
    {
        // Convert milliseconds to microseconds
        int usecs = timeout_ms * 1000;

        // Determine the timeout in seconds and microseconds
        timeout.tv_sec  = usecs / 1000000;
        timeout.tv_usec = usecs % 1000000;

        // Point to the timeout structure we just initialized
        pTimeout = &timeout;
    }

    // We'll wait on input from the file descriptor
    FD_ZERO(&rfds);
    FD_SET(m_fd, &rfds);

    // Wait for a character to be available for reading
    int status = select(m_fd+1, &rfds, NULL, NULL, pTimeout);

    // If status > 0, there is a character ready to be read
    return (status > 0);
}
//============================================================================


//============================================================================
// drain_input() - Drains all data from the serial port and throws it away
//============================================================================
void CSerialPort::drain_input(int timeout_ms)
{
    char c;

    // Read in and throw away data until the line goes quiet for awhile
    while (data_is_available(timeout_ms)) bitbucket = ::read(m_fd, &c, 1);
}
//============================================================================


//============================================================================
// get_line() - Fetches a line from the serial port.   Throws away carriage
//              returns and strips off the terminating line-feed
//============================================================================
bool CSerialPort::get_line(void* buffer, int timeout_ms)
{
    // Convert "buffer" to a char*
    char* out = (char*) buffer;

    // We're going to read bytes until we encounter a line-feed...
    while (true)
    {
        // Read a single character from the serial port using get_char
        int c = get_char(timeout_ms);

        // If a timeout occured, tell the caller
        if (c == -1) return false;

        // If it's a carriage return, throw it away
        if (c == '\r') continue;

        // It's a line feed, we've hit the end of the line
        if (c == '\n') break;

        // Append this character to our result string
        *out++ = c;
    }

    // Terminate the line with a nul
    *out = 0;

    // Tell the caller that we retreived a line of text
    return true;
}
//============================================================================



//============================================================================
// printf() - Sends printf()-style data to the serial port
//============================================================================
void CSerialPort::printf(const char* fmt, ...)
{
    char      buffer[1000];
    va_list   args;

    // Get the pointer to the first argument
    va_start(args, fmt);

    // Fill "buffer" with the printf output
    vsnprintf(buffer, sizeof buffer, fmt, args);

    // We're done with the pointer to the first argument
    va_end(args);

    // Write the resulting string to the serial port
    write(buffer, strlen(buffer));

}
//============================================================================



//============================================================================
// put_line() - Sends a line of text to the serial port.
//============================================================================
void CSerialPort::put_line(const void* text)
{
    // Send a the text line to the serial port
    write(text, strlen((char*)text));
}
//============================================================================


//============================================================================
// get_char() - Fetches one byte from the serial port
//
// Passed:  timeout_ms = The number of milliseconds to wait for a character
//                       to be available on the serial port.  -1 means
//                       "wait forever"
//
// Returns: A character that we read from the serial port
//               --OR--
//          A -1 to indicate that no character was available to read
//============================================================================
int CSerialPort::get_char(int timeout_ms)
{
    unsigned char c;

    // Wait for a character to be available for reading, and if one doesn't
    // arrive within the specified timeout, tell the caller that a timeout
    // occured.
    if (!data_is_available(timeout_ms)) return -1;

    // Read a single character from the serial port
    bitbucket = ::read(m_fd, &c, 1);

    // If we are supposed to display our output, do so
    if (m_sniff) ::printf("%c", c);

    // Hand the caller the character we just read from the serial port
    return c;
}
//============================================================================


//============================================================================
// put_char() - Writes a single byte to the serial port
//============================================================================
void CSerialPort::put_char(int byte)
{
    unsigned char c = byte;
    write(&c, 1);
}
//============================================================================


//============================================================================
// read() - Reads a specified number of bytes from the serial port
//============================================================================
bool CSerialPort::read(void* buffer, int count, int timeout_ms)
{
    // Convert the input buffer into a char*
    char* out = (char*) buffer;

    // Read as many characters as were specified by the caller...
    while (count--)
    {
        // Fetch a character from the serial port
        int c = get_char(timeout_ms);

        // If we timed out, tell the caller
        if (c == -1) return false;

        // Store the character we just read into the caller's buffer
        *out++ = c;
    }

    // Tell the caller that we read in all the data he wanted
    return true;
}
//============================================================================


//============================================================================
// write() - Writes a specified number of bytes to the serial port
//============================================================================
void CSerialPort::write(const void* buffer, int count)
{
    bitbucket = ::write(m_fd, buffer, count);

    // If we're sniffing...
    if (m_sniff)
    {
        // Point to the characters that we wrote to the serial port
        char* in = (char*) buffer;

        // Display them
        while (count--) ::printf("%c", *in++);

        // And make sure they get displayed
        fflush(stdout);
    }

}
//============================================================================



