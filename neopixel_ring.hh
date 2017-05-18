#pragma once
#include "serial.hh"
#include "animations.hh"

class NeopixelComms final : public serial::CommunicationBase
{
public: // destructor /////////////////////////////////////////////////////////
    //
    //
    //
    NeopixelComms(const serial::SerialConnection &s);

    //
    //
    //
    ~NeopixelComms() {};

public: // types //////////////////////////////////////////////////////////////
    enum bits : unsigned char
    {
        ZERO = 0xC0,
        ONE = 0xF8
    };

public: // methods ////////////////////////////////////////////////////////////
    //
    // Send a frame to the neopixel display one byte at a time
    //
    void send_frame(const animations::Frame &f);

private: // methods ///////////////////////////////////////////////////////////
    //
    // Take a byte in and return a funky SPI formatted ByteVector.
    // Since the Neopixel communicates in a weird protocol, this
    // conversion is required.
    //
    ByteVector_t convert_byte_to_spi(const BYTE &byte);

};

