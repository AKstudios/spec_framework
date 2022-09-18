#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <string>
#include <string.h>
#include "cmd_server_base.h"
#include "tokenizer.h"
using namespace std;


static int    s_to_i(const string& s)  {return strtol(s.c_str(), NULL, 0);}
static double s_to_d(const string& s)  {return strtod(s.c_str(), NULL   );}

static CTokenizer tokenizer;



//==========================================================================================================
// make_lower() - Converts a std::string to lower-case
//==========================================================================================================
static void make_lower(string& s)
{
    char* in = (char*)s.c_str();
    while (*in)
    {
        if (*in >= 'A' && *in <= 'Z') *in |= 32;
        ++in;
    }
}
//==========================================================================================================


//==========================================================================================================
// convert_tabs_to_spaces() - Changes every tab to a space
//
// Returns 'true' if the input line contains non-whitespace data
//==========================================================================================================
static bool convert_tabs_to_spaces(char* in)
{
    // For a moment, assume this line consists <only> of spaces and tabs
    bool line_has_nonspace_data = false;
    
    // Loop through each character of the line
    while (*in)
    {
        // If this character is a tab, make it a space
        if (*in == 9) *in = ' ';

        // Keep track of whether or not this line is blank
        if (*in != ' ') line_has_nonspace_data = true;
        
        // And point to the next character in the line
        ++in;
    }

    // Tell the caller whether there is valid data on this line
    return line_has_nonspace_data;
}
//==========================================================================================================


//==========================================================================================================
// get_first() - Returns the first token of the tokenized command string
//==========================================================================================================
string server_command_t::get_first(bool force_lower)
{
    string cmd;

    // If the m_tokens vector isn't empty, fetch the first token
    if (m_tokens.size()) cmd = m_tokens[0];

    // If we're supposed to force it to lower-case, do so
    if (force_lower) make_lower(cmd);

    // After a "get_first()", the next subsequent call to get_next() should return the first parameter
    m_next_index = 1;

    // Hand the caller the command
    return cmd;
}
//==========================================================================================================


//==========================================================================================================
// get_next() - Fetches the next available paramter.
//
// Passed:  p_result    = Pointer to field where result should be stored
//          force_lower = The result will be forced to lower-case if this is true
//
// Returns: true if there was a token available to fetch, otherwise false
//==========================================================================================================
bool server_command_t::get_next(string* p_result, bool force_lower)
{
    // If we're out of tokens, inform the caller
    if (m_next_index >= m_tokens.size())
    {
        *p_result = "";
        return false;
    }

    // Fetch the next available token    
    string token = m_tokens[m_next_index++];

    // If we've been asked to force it to lower-case, do so
    if (force_lower) make_lower(token);

    // Store the token into the caller's output field
    *p_result = token;

    // Tell the caller that he has a valid token
    return true;
}
//==========================================================================================================


//==========================================================================================================
// get_next() - Fetches the next available paramter as a double-float
//
// Passed:  p_result  = Pointer to field where result should be stored
//
// Returns: true if there was a token available to fetch, otherwise false
//==========================================================================================================
bool server_command_t::get_next(double *p_result)
{
    string  token;

    // If there are no more tokens available, tell the caller
    if (!get_next(&token, false))
    {
        *p_result = 0;
        return false;
    }

    // Otherwise, fill in the caller's output field
    *p_result = s_to_d(token);

    // Tell the caller that he has a valid token
    return true;
}
//==========================================================================================================


//==========================================================================================================
// get_next() - Fetches the next available paramter as an integer
//
// Passed:  p_result = Pointer to field where result should be stored
//
// Returns: true if there was a token available to fetch, otherwise false
//
// Notes:   the input token may be in either decimal, or in hex with the "0x" prefix
//==========================================================================================================
bool server_command_t::get_next(int *p_result)
{
    string  token;

    // If there are no more tokens available, tell the caller
    if (!get_next(&token, false))
    {
        *p_result = 0;
        return false;
    }

    // Otherwise, fill in the caller's output field
    *p_result = s_to_i(token);

    // Tell the caller that he has a valid token
    return true;
}
//==========================================================================================================



//==========================================================================================================
// get_next() - Fetches the next available paramter as an unsigned integer
//
// Passed:  p_result = Pointer to field where result should be stored
//
// Returns: true if there was a token available to fetch, otherwise false
//
// Notes:   the input token may be in either decimal, or in hex with the "0x" prefix
//==========================================================================================================
bool server_command_t::get_next(unsigned int *p_result)
{
    string  token;

    // If there are no more tokens available, tell the caller
    if (!get_next(&token, false))
    {
        *p_result = 0;
        return false;
    }

    // Otherwise, fill in the caller's output field
    *p_result = s_to_i(token);

    // Tell the caller that he has a valid token
    return true;
}
//==========================================================================================================




//==========================================================================================================
// main() - This is the entry point of the new thread when it spawns
//==========================================================================================================
void CCmdServerBase::main()
{

wait_for_connection:

    char buffer[512];

    // If we're in verbose mode, tell the user that we're creating a server
    if (m_verbose) printf("Creating server on port %i\n", m_port);;

    // Create the server
    if (!m_socket.create_server(m_port))
    {
        fprintf(stderr, "Can't create server on port %i", m_port);
        exit(1);
    }

    // Our server thread is now fully initialized
    m_is_initialized = true;

    // Wait for a client to connect to us
    m_socket.listen_and_accept();

    // We we send data, we want it transmitted immediately
    m_socket.set_nagling(false);

    // When we get here, a client has connected to us
    m_is_connected = true;

    // If we're in verbose mode, tell the user that we have a connection
    if (m_verbose) printf("A client has connected from %s\n", m_socket.get_peer_address().c_str());

    // Let the derived class have a chance to know that a client just connected
    on_client_connect();

    // While the socket is open, read and handle input lines
    while (m_socket.getline(buffer, sizeof buffer))
    {
        // Convert tabs to spaces, and ignore the line if it's blank
        if (!convert_tabs_to_spaces(buffer)) continue;

        // If we're in verbose mode, show the user the line we just received
        if (m_verbose) printf(">> %s\n", buffer);

        // Parse the buffer into a vector of tokens
        m_line = tokenizer.parse(buffer);

        // Go handle this command
        handle_command();
    }

    // If we get here, the client has closed the connection
    m_is_connected = false;

    // The socket has closed.   If we're in verbose mode, tell the user
    if (m_verbose) printf("Socket closed by client\n");
    
    // And go back to waiting for a connection
    goto wait_for_connection;
}
//==========================================================================================================


//==========================================================================================================
// start() - Starts the server
//==========================================================================================================
void CCmdServerBase::start(cmd_server_t* p_params)
{
    // Fill in the parameter fields
    m_verbose = p_params->verbose;
    m_port    = p_params->port; 

    // And spawn the server
    spawn();   
}
//==========================================================================================================


//==========================================================================================================
// send() - Sends data to the other side of the connection
//==========================================================================================================
void CCmdServerBase::send(const char* buffer, int length)
{
    // Mutexes must exist statically, not on the stack
    static mutex mtx;

    // If the caller didn't specify a length, compute it
    if (length == -1) length = strlen(buffer);

    // Don't send empty strings
    if (length == 0) return;

    // If there's not a client connected, don't bother trying
    if (!m_is_connected) return;

    // If we're in verbose mode, show the user what we're sending
    if (m_verbose) printf("<< %s", buffer);

    // Ensure that only one thread at a time can write to the socket
    mtx.lock();
    
    // Send the string to the connected client
    m_socket.send(buffer, length);

    // Allow other threads to send data on over the socket
    mtx.unlock();
}
//==========================================================================================================



//==========================================================================================================
// sendf() - Sends a printf-style formatted data to the the other side of a connected socket
//
// Returns either : -1 = An error occured
//                  Anything else = the number of bytes actually sent.  The entire string will always be
//                  sent unless the socket was closed by the other side
//==========================================================================================================
void CCmdServerBase::sendf(const char* fmt, ...)
{
    char buffer[1000];

    // This is a pointer to the variable argument list
    va_list ap;

    // Point to the first argument after the "fmt" parameter
    va_start(ap, fmt);

    // Perform a printf of our arguments into the buffer area;
    vsnprintf(buffer, sizeof buffer, fmt, ap);

    // Tell the system that we're done with the "ap"
    va_end(ap);

    // And send the buffer
    send(buffer, strlen(buffer));
}
//==========================================================================================================





//==========================================================================================================
// pass() - Reports OK (with optional parameters) to the client
//==========================================================================================================
bool CCmdServerBase::pass(const char* fmt, ...)
{
    va_list   ap;
    char      buffer[1000] = "OK";

    // Point to the position immediately after the OK
    char* out = buffer + 2;

    // If the caller gave us parameters to send...
    if (fmt)
    {
        // Add a space after the "OK"
        *out++ = ' ';

        // This accounts for "OK<space>\r\n\0"
        int bytes_left_in_buffer = sizeof(buffer) - 6;

        // Point to the first argument after the "fmt" parameter
        va_start(ap, fmt);

        // Perform a printf of our arguments into the buffer area
        vsnprintf(out, bytes_left_in_buffer, fmt, ap);

        // Tell the system that we're done with the "ap"
        va_end(ap);

        // Find the nul-byte at the end of the buffer
        out = strchr(buffer, 0);
    }
    
    // Add a carriage return and linefeed
    *out++ = '\r';
    *out++ = '\n';
    *out   = 0;

    // And send that string to the client
    send(buffer);

    // This function always returns true
    return true;
}
//==========================================================================================================



//==========================================================================================================
// fail() - Reports a failure (with optional parameters) to the client
//==========================================================================================================
bool CCmdServerBase::fail(const char* failure, const char* fmt, ...)
{
    char      buffer[1000];
    va_list   ap;
    
    // The prefix now contains "FAIL <failure><nul-byte>"
    snprintf(buffer, sizeof buffer, "FAIL %s", failure);

    // Point to the position immediately after the failure message
    char* out = strchr(buffer, 0);

    // Find out how much room we have left in the buffer
    int bytes_left_in_buffer = out - buffer - 2;

    // If the caller gave us parameters to send...
    if (fmt)
    {
        // Add a space after the prefix
        *out++ = ' ';
        --bytes_left_in_buffer;

        // Point to the first argument after the "fmt" parameter
        va_start(ap, fmt);

        // Perform a printf of our arguments into the buffer area
        vsnprintf(out, bytes_left_in_buffer, fmt, ap);

        // Tell the system that we're done with the "ap"
        va_end(ap);

        // Find the nul-byte at the end of the buffer
        out = strchr(buffer, 0);
    }
   
    // Add a carriage return and linefeed
    *out++ = '\r';
    *out++ = '\n';
    *out   = 0;

    // And send that string to the client
    send(buffer);

    // This function always returns false
    return false;
}
//==========================================================================================================

