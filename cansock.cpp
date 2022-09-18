//==========================================================================================================
// CANSock.cpp - Implements a class for sending and receiving data over a CAN interface
//==========================================================================================================
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include "cansock.h"
#include "netutil.h"
using namespace std;

static volatile int bit_bucket;

//==========================================================================================================
// close() - Closes the socket connection to the CAN interface
//==========================================================================================================
void CANSock::close()
{
    if (m_sd >= 0) ::close(m_sd);
    m_sd = -1;    
}
//==========================================================================================================



//==========================================================================================================
// connect() - Connects to the specified CAN interface
//==========================================================================================================
bool CANSock::connect(string interface)
{
    ifreq ifr;

    union
    {
        sockaddr sa;
        sockaddr_can addr;        
    };
    
    // Fetch the const char* to the name of the interface
    const char* iface = interface.c_str();

    // Ensure that the interface name passed to us won't overlow the buffer
    if (strlen(iface) >= IFNAMSIZ) return false;

    // Open the CAN socket
    m_sd = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    // If we can't open the socket, tell the caller we failed
    if (m_sd == -1) return false;

    // Copy our interface name into the interface structure
	strcpy(ifr.ifr_name, iface);
	
    // Fetch the index that is associated with this interface name
    if (ioctl(m_sd, SIOCGIFINDEX, &ifr) < 0) return false;

    // Fill in the "addr" structure with the index of our CAN interface
	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

    // Bind our socket to the index of this interface
    if (bind(m_sd, &sa, sizeof(addr)) < 0) return false;

    // If we get here, we have a valid connection to the CAN interface
    return true;
}
//==========================================================================================================



//==========================================================================================================
// put() - Places a message onto the CAN bus
//==========================================================================================================
void CANSock::put(int msg_id, const void* buffer, size_t buf_size)
{
    can_frame frame;

    // If the caller gave us too much data, do nothing
    if (buf_size > 8) return;

    // Fill in the CAN frame structure
    frame.can_id  = msg_id;
    frame.can_dlc = buf_size;
    memcpy(frame.data, buffer, buf_size);

    // Send the data to the CAN bus and tell the caller if it worked
    bit_bucket = ::write(m_sd, &frame, sizeof frame);
}
//==========================================================================================================


//==========================================================================================================
// get() - Fetches the next available message from the CAN bus
//==========================================================================================================
bool CANSock::get(can_frame* p_frame, int timeout_ms)
{
    // Wait for data to arrive.   If we timeout, tell the caller
    if (!NetUtil::wait_for_data(timeout_ms, m_sd)) return false;

    // Read the CAN frame from the interface
    bit_bucket = read(m_sd, p_frame, sizeof(can_frame));
    
    // And tell the caller that his frame structure has a valid frame
    return true;
}
//==========================================================================================================

