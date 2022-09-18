//==========================================================================================================
// mgmt_server.cpp - Implements a process-management UDP server
//==========================================================================================================
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "mgmt_server.h"
#include "udpsock.h"

enum
{
    CMD_PING = 1,
    RSP_PING = 2,
    CMD_DOWN = 3
};

// Message structures for the message we understand
struct cmd_ping_t {uint16_t cmd; uint16_t port;};
struct rsp_ping_t {uint16_t cmd; uint16_t port;};

//==========================================================================================================
// main() - Performs process management
//==========================================================================================================
void CMgmtServer::main()
{
    int reply_port;

    union 
    {
        char        buffer[4];
        uint16_t    cmd;
        cmd_ping_t  ping_cmd;
        rsp_ping_t  ping_rsp;
    };

    // The UDP client and server socket
    UDPSock client, server;

    // The UDP port we want to listen on is in the first spawn parameter
    uint16_t port = *(int*)(m_spawn_param[0]);

    // Create a UDP server
    if (!server.create_server(port, "", AF_INET))
    {
        printf("Can't create management server on socket %d\n", port);
        while(true) sleep(999999999);
    }

    // Wait for process management messages to arrive
    while (server.receive(buffer, sizeof buffer))
    {
        // If the command was "ping"...
        if (cmd == CMD_PING)
        {
            // The ping message includes the port number to reply to
            reply_port = ping_cmd.port;
            
            // We're going to send a ping response that identifies who we are
            ping_rsp = {RSP_PING, port};

            // Create a sender socket
            client.create_sender(reply_port, "localhost", AF_INET);
            
            // Send the ping response
            client.send(&ping_rsp, sizeof ping_rsp);
            
            // We're done with that socket
            client.close();
            
            // And go back to listening for more messages
            continue;
        }        

        // If the command was "down", exit the program
        if (cmd == CMD_DOWN) exit(0);
    }
};
//==========================================================================================================
