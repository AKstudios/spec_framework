//=========================================================================================================
// mstimer.h - Defines a rollover-safe one-shot timer and a rollover-safe repeating timer
//=========================================================================================================
#pragma once
#include <stdint.h>

//--------------------------------------------------------------------------------------------------------
// This is a rollover-safe repeating timer, and the base-class for a one-shot timer
//--------------------------------------------------------------------------------------------------------
class msTimer
{
public:

    // Returns the number of milliseconds since power up
    static uint64_t millis();

    // Constructor : timer is instantiated in the "stopped" condition
    msTimer() { m_is_running = false; }

    // Call this to start or restart the timer
    void    start(uint64_t duration_ms);

    // call this to stop the timer
    void    stop() {m_is_running = false;}

    // Call this to find out if the timer is running and expired.
    bool    is_expired();

    // check if the timer is still running
    bool    is_running() {return m_is_running;}

protected:


    // The is the timerstamp when the timer was started
    uint64_t    m_start_time;

    // This is the duration of the one-shot timer in milliseconds
    uint64_t    m_duration_ms;

    // This is true if the timer is running 
    bool        m_is_running;
};


//--------------------------------------------------------------------------------------------------------
// This is a rollover-safe OneShot timer
//--------------------------------------------------------------------------------------------------------
class OneShot : public msTimer
{
public:

    // Constructor
    OneShot() { m_is_kicked = false; }

    // Call this to find out if the timer is running and expired.
    // If this returns 'true', the timer has been stopped
    bool    is_expired();

    // Restarts the timer in a thread-safe manner
    void    kick() { m_is_kicked = true; }

protected:

    // When this is true, the call to "is_expired" will restart the timer
    bool    m_is_kicked;

};
//--------------------------------------------------------------------------------------------------------


