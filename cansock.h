//==========================================================================================================
// CANSock.h - Defines a class for sending and receiving data over a CAN interface
//==========================================================================================================
#pragma once
#include <sys/socket.h>
#include <linux/can.h>
#include <string>

/*
<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
  To use this on a machine that doesn't have a native CAN interface, you can create a virtual CAN interface
  by running this script.  The virtual CAN interface created will exist until next reboot.

  You should install can-utils (i.e., sudo apt install can-utils) so you can use the candump sniffer.

  To use can-dump (after installing it), run "candump vcan0"
<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>

modprobe can
modprobe can_raw
modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
ip link show vcan0

<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
*/



class CANSock
{
public:
    
    // Default constuctor
    CANSock() {m_sd = -1;}

    // Default destructor closes the interface connection
    ~CANSock() {close();}

    // Call this to connect to a CAN interface
    bool    connect(std::string interface);

    // Closes the connection to the CAN interface
    void    close();

    // Call this to place a message onto the CAN bus. 'buffer' must be 8 bytes or less
    void    put(int msg_id, const void* buffer, size_t buf_size);

    // Call this to fetch the next message from the CAN bus.  Timeout of -1 means "wait forever"
    bool    get(can_frame* p_frame, int timeout_ms = -1);

protected:

    // The socket descriptor
    int     m_sd;
};




