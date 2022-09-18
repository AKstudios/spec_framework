//==========================================================================================================
// endian.h - Describes structures that represent big-endian and little-endian integers
//==========================================================================================================
#pragma once
#include <inttypes.h>

// Big endian 64-bit integer
struct be_uint64_t
{
    // Storage
    unsigned char octet[8];

    // Cast to a uint64_t
    operator uint64_t() {return get();}

    // Assignment from a uint64_t
    be_uint64_t& operator=(uint64_t rhs) {set(rhs); return *this;}

    // Get the 64-bit value
    uint64_t get()
    {
        uint64_t result = octet[0];
        for (int i=1; i<8; ++i) result = (result << 8) | octet[i];
        return result;        
    }
    
    // Set the 64-bit value
    void set(uint64_t v)
    {
        for (int i=7; i>=0; --i)
        {
            octet[i] = v;
            v >>= 8;
        }        
    }

};



// Big endian 32-bit integer
struct be_uint32_t
{
    // Storage
    unsigned char octet[4];

    // Cast to a uint32_t
    operator uint32_t() {return get();}

    // Assignment from a uint32_t
    be_uint32_t& operator=(uint32_t rhs) {set(rhs); return *this;}

    // Get and set the value
    uint32_t get() {return (octet[0] << 24 | octet[1] << 16 | octet[2] << 8 | octet[3]);}
    void     set(uint32_t v) {octet[0] = v >> 24; octet[1] = v >> 16; octet[2] = v >> 8; octet[3] = v;}
};


// Big endian 16-bit integer
struct be_uint16_t
{
    // Storage
    unsigned char octet[2];

    // Cast to a uint16_t
    operator uint16_t() {return get();}

    // Assignment from a uint16_t
    be_uint16_t& operator=(uint16_t rhs) {set(rhs); return *this;}

    // Get and set the value
    uint16_t get() {return (octet[0] << 8 | octet[1]);}
    void     set(uint16_t v) {octet[0] = v >> 8; octet[1] = v;}
};


// Little endian 32-bit integer
struct le_uint32_t
{
    // Storage
    unsigned char octet[4];

    // Cast to a uint32_t
    operator uint32_t() {return get();}

    // Assignment from a uint32_t
    le_uint32_t& operator=(uint32_t rhs) {set(rhs); return *this;}

    // Get and set the value
    uint32_t get() {return (octet[3] << 24 | octet[2] << 16 | octet[1] << 8 | octet[0]);}
    void     set(uint32_t v) {octet[3] = v >> 24; octet[2] = v >> 16; octet[1] = v >> 8; octet[0] = v;}
};


// Little endian 16-bit integer
struct le_uint16_t
{
    // Storage
    unsigned char octet[2];

    // Cast to a uint16_t
    operator uint16_t() {return get();}

    // Assignment from a uint16_t
    le_uint16_t& operator=(uint16_t rhs) {set(rhs); return *this;}

    // Get and set the value
    uint16_t get() {return (octet[1] << 8 | octet[0]);}
    void     set(uint16_t v) {octet[1] = v >> 8; octet[0] = v;}
};
