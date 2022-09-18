#include <unistd.h>
#include <sys/time.h>
#include "mstimer.h"

//=========================================================================================================
// start() - Starts or restarts the timer
//=========================================================================================================
void msTimer::start(uint64_t duration_ms)
{
    m_duration_ms = duration_ms;
    m_start_time  = millis();
    m_is_running  = true;
}
//=========================================================================================================


//=========================================================================================================
// is_expired() - Returns true if our timer was running and has expired
//=========================================================================================================
bool msTimer::is_expired()
{
    // A timer that isn't running is by definition not expired
    if (!m_is_running) return false;

    // Fetch the current timestamp
    uint64_t now = millis();

    // How many milliseconds have elapsed since the timer was started?
    uint64_t elapsed = now - m_start_time;

    // Is the timer expired?
    bool expired = elapsed >= m_duration_ms;

    // If the timer hasn't yet expired, tell the caller
    if (!expired) return false;

    //---------------------------------------------------
    // If we get here, we're restarting an expired timer
    //---------------------------------------------------

    // Compute the timestamp when the timer restarted
    m_start_time += m_duration_ms;
            
    // Compute the number of milliseconds that have elapsed since the new start time
    elapsed = now - m_start_time;

    // If the restarted timer is <already> expired, it restarts right now
    if (elapsed > m_duration_ms) m_start_time = now;

    // Tell the caller that the timer expired
    return true;
}
//=========================================================================================================





//=========================================================================================================
// is_expired() - Returns true if our timer was running and has expired
//=========================================================================================================
bool OneShot::is_expired()
{
    // A timer that isn't running is by definition not expired
    if (!m_is_running) return false;

    // Fetch the current timestamp
    uint64_t now = millis();

    // If the timer has been kicked, restart it
    if (m_is_kicked)
    {
        m_is_kicked = false;
        m_start_time = now;
        return false;
    }

    //------------------------------------------------------------------------------------
    // If we get here, the timer is running.  Now we need to determine whether the timer
    // has exceeded its duration and should therefore stop running. (i.e., whether it has
    // expired)
    //------------------------------------------------------------------------------------
    
    // How many milliseconds have elapsed since the timer was started?
    uint64_t elapsed = now - m_start_time;

    // Should the timer continue running?
    m_is_running = elapsed < m_duration_ms;

    // Tell the caller whether or not this timer is expired
    return !m_is_running;
}
//=========================================================================================================



//=========================================================================================================
// millis() - Returns a timestamp with millisecond resolution.   This is NOT monotonic, and can 
//            "travel back in time" if the system time is changed!
//=========================================================================================================
uint64_t msTimer::millis()
{
    struct timeval ts;

    // Fetch the time since the eppch
    gettimeofday(&ts, NULL);

    // Return the number of milliseconds elapsed since the epoch
    return (uint64_t)(ts.tv_sec) * 1000 + (uint64_t)(ts.tv_usec) / 1000;
}
//=========================================================================================================
