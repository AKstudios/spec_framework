//==========================================================================================================
// cmd_server_base.h - Defines a base-class for a TCP command server
//==========================================================================================================
#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "cthread.h"
#include "netsock.h"

//----------------------------------------------------------------------------------------------------------
// One of these must be passed to the "start()" routine
//----------------------------------------------------------------------------------------------------------
struct cmd_server_t
{
    bool verbose;
    int  port;    
};
//----------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------
// This is a vector of strings with some helper functions attached.
//
// These objects are fully copyable
//----------------------------------------------------------------------------------------------------------
class server_command_t
{
public:

    // Default constructor
    server_command_t() {m_next_index = 1;}

    // Constructor from a string vector
    server_command_t(const std::vector<std::string> rhs) {fill_tokens(rhs);}

    // Copy constructor
    server_command_t(const server_command_t& rhs) {fill_tokens(rhs.m_tokens);}

    // Overloading the '=' operator so we can assign a string vector
    void  operator=(const std::vector<std::string> rhs) {fill_tokens(rhs);}

    // Call this to fetch the command token
    std::string get_first(bool force_lower = true);

    // Call one of these to fetch the next command parameter
    bool        get_next(std::string  *p_result, bool force_lower = true);
    bool        get_next(double       *p_result);
    bool        get_next(int          *p_result);
    bool        get_next(unsigned int *p_result);

    // Returns the number of parameters
    int         param_count() {return m_tokens.size() - 1;}

protected:

    // Call this to fill in the tokens array
    void fill_tokens(const std::vector<std::string>& rhs) {m_tokens = rhs, m_next_index = 1;}

    // This is the index of the next item to be retrieved via get_next())
    int m_next_index;

    // The tokens
    std::vector<std::string> m_tokens;

};
//----------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------
// CCmdServerBase - Serves as the generic base class for TCP command servers
//----------------------------------------------------------------------------------------------------------
class CCmdServerBase : public CThread
{
public:

    // Constructor
    CCmdServerBase() : CThread() {m_is_connected = m_is_initialized = false;}

    // Call this to start the server
    void    start(cmd_server_t *params);

    // Sends data to the other side of the connection
    void    send(const char* buffer, int length = -1);
    void    sendf(const char* fmt, ...);

    // Returns 'true' if the server is fully initialized and running
    bool    is_initialized() {return m_is_initialized;}

    // Returns 'true' if there is a client connected to this server
    bool    is_connected() {return m_is_connected;}

protected:

    // Over-ride this to handle commands
    virtual void handle_command() {}

protected:

    // This is the entry point when the server thread spawns
    void    main();

    // This gets called whenever a client connects
    virtual void  on_client_connect() {};

    // When handle_command() gets called, the command will be in m_line
    server_command_t m_line;

    // Call this to report an "ok" result to the client
    bool    pass(const char* fmt = NULL, ...);

    // Call this to report a "fail" result to the client
    bool    fail(const char* failure, const char* fmt = NULL, ...);

    // This is a convenience method for reporting a syntax error
    bool    fail_syntax() {send("FAIL syntax\r\n"); return false;}

    // This will be true once the server thread is initialized and fully functional
    bool    m_is_initialized;

    // This will be true when there is a client connected
    bool    m_is_connected;

    // This is the TCP port we should listen on
    int     m_port;

    // This will be true if we want verbose output for debugging
    bool    m_verbose;

private:

    // This is the network socket that will be our server socket
    NetSock m_socket;
};
//----------------------------------------------------------------------------------------------------------