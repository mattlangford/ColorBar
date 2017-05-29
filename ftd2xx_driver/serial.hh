#pragma once
#include "ftd2xx.h"
#include <memory>
#include <vector>

namespace serial
{

//
// Some fancy MPSSE (Multi-Protocol Synchronous Serial Engine) Commands
// This is a subset of the entire set, since I'm only adding them as I use them
//
enum mpsse : unsigned char
{
    MSB_R_EDGE_OUT_BYTE          = 0x10, // 0xLENGTH_L, 0xLENGTH_H, 0xDATA...  - on pin D0
    MSB_R_EDGE_OUT_BIT           = 0x13, // 0xLENGTH_L, 0xLENGTH_H, 0xDATA...  - on pin D0
    SET_D_BUS_DATA               = 0x80, // 0xVALUE, 0xDIRECTION (1 = output)
    GET_D_BUS_DATA               = 0x81,
    SET_C_BUS_DATA               = 0x82, // 0xVALUE, 0xDIRECTION (1 = output)
    GET_C_BUS_DATA               = 0x83,
    LOOPBACK_ENABLE              = 0x84,
    LOOPBACK_DISABLE             = 0x85,
    SET_TCK_DIVISOR              = 0x86, // VAL_L, VAL_H
    CLOCK_60_MHZ                 = 0x8A,
    THREE_PHASE_CLOCKING_ENABLE  = 0x8C,
    THREE_PHASE_CLOCKING_DISABLE = 0x8D,
    ADAPTIVE_CLOCKING_ENABLE     = 0x96,
    ADAPTIVE_CLOCKING_DISABLE    = 0x97,
    BAD_COMMANDS                 = 0xFA
};

//
// Public type used by others when writing data to the board
//
using ByteVector_t = std::vector<BYTE>;

//
// Connects to an FTDI serial connection and has some nice wrappers C++11 around the
// gross C
//
class SerialConnection
{
public: // constructor ////////////////////////////////////////////////////////
    SerialConnection(const unsigned int device_number = 0);

    SerialConnection(const SerialConnection &s);

    ~SerialConnection();

public: // publicest methods //////////////////////////////////////////////////
    //
    // Simple c++11 wrapper to write some data and check that it went through.
    // Give it a byte vector that you don't care about since it'll eat it
    //
    bool write_data(ByteVector_t data) const;

    //
    // Wait until we have some number of bytes in the receive buffer,
    // read them, clear them, and return the data
    // TODO: Timeouts
    //
    ByteVector_t block_and_read(const unsigned int num_bytes_to_read) const;

    //
    // SPI command to send bytes out on D0 (which is D bus pin 1)
    //
    bool spi_write_data(ByteVector_t data) const;

    //
    // Request the data that is on a pin. According to the documentation, there are two
    // pin sets: D and C. D is the lower byte and C is the upper byte. The Pin number
    // here will start at 0 for D0 and end at 15 for C7. C8 and C9 are reserved
    //
    // TODO: There could be a more efficient implementation that could check multiple
    //       pins at once, but I don't plan to use this for much
    //
    bool get_pin(const size_t pin_number) const;

    //
    // Set a pin with the same rules as above
    //
    void set_pin(const size_t pin_number, bool high) const;

public: // more public methods ////////////////////////////////////////////////
    //
    // Sets default values for pins and what not - maybe this should go in the
    // constructor?
    //
    void configure_spi_defaults() const;

    //
    // Basic test script - sends some bad data and ensures it gets an error back
    // Returns true if everything is working, false otherwise
    //
    void run_comms_check() const;

private: // methods ///////////////////////////////////////////////////////////
    //
    // Makes sure the status return FT_OK
    //
    inline bool status_okay(const FT_STATUS ft_status) const;

    //
    // Checks for the bad response code and alerts the user
    //
    inline void check_bad_response(const ByteVector_t &recv_buffer) const;

private: // members ///////////////////////////////////////////////////////////

    //
    // Handle to the device
    //
    FT_HANDLE ft_handle;

};


using SerialConnection_ptr = std::shared_ptr<SerialConnection>;


} // namespace serial
