//============================================================================
// serial_port.h - Defines an API for raw serial I/O services
//============================================================================
#pragma once
#include <termios.h>
#include <string>
#include <stdint.h>
#include <sys/select.h>

//============================================================================
// Handy constants used for describing timeout values
//============================================================================
#define SP_DEFAULT_TIMEOUT -2
#define SP_NO_TIMEOUT      -1
//============================================================================


//============================================================================
// Class CSerialPort - Provides an API to a UART
//============================================================================
class CSerialPort
{
public:

    // Constructor and destructor
    CSerialPort();
    ~CSerialPort();

    // Call this to set the default timeout for functions that read data
    void    set_default_read_timeout(int milliseconds);

    // Call this to open a connection.  Returns 'false' on error
    bool    open(std::string device, uint32_t baud);

    // Call this to close a connection
    void    close();

    // Throws away data coming from the serial port
    void    drain_input(int timeout_ms);

    // Writes a line of text to the serial port. Caller must append
    // carriage return or line feed if needed
    void    put_line(const void* line);

    // Writes a line of printf()-style text to the serial port.  Caller
    // appends cr/lf if needed
    void    printf(const char* fmt, ...);

    // Fetches a line of text from the serial port. Strips cr/lf off the end
    bool    get_line(void* buffer, int timeout_ms = SP_DEFAULT_TIMEOUT);

    // Call this to fetch the file descriptor of the UART
    int     get_fd() {return m_fd;}

    // Fetches one character from the serial port
    int     get_char(int timeout_ms = SP_DEFAULT_TIMEOUT);

    // Puts a single character to the serial port
    void    put_char(int byte);

    // Reads a specified number of bytes from the serial port
    bool    read(void* buffer, int count, int timeout_ms = SP_DEFAULT_TIMEOUT);

    // Writes a specified number of bytes from the serial port
    void    write(const void* buffer, int count);

    // Enable sniffing
    void    enable_sniffing(bool flag) {m_sniff = flag;}

protected:

    // This returns 'true' if data is available to be read in.
    // If timeout_ms = -1, this routine will wait forever for data to
    // be available
    bool    data_is_available(int timeout_ms);

    // Converts an integer baud-rate to one of the termios speed constants
    speed_t baud_to_constant(uint32_t baud_rate);

    // File descriptor we use to read/write serial data
    int     m_fd;

    // If this is 'true', all incoming characters will be printed
    bool    m_sniff;

    // This is the default timeout in milliseconds
    int     m_default_timeout_ms;
};
//============================================================================


