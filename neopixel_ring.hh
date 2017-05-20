#pragma once
#include "serial.hh"
#include "animations.hh"

class NeopixelComms final : public serial::CommunicationBase
{
public: // destructor /////////////////////////////////////////////////////////
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
    ByteVector_t build_frame(const animations::Frame &f);

private: // methods ///////////////////////////////////////////////////////////
    //
    // Take a byte in and return a funky SPI formatted ByteVector.
    // Since the Neopixel communicates in a weird protocol, this
    // conversion is required.
    //
    ByteVector_t convert_byte_to_spi(const BYTE &byte);

};

class PythonController
{
public: // constructor ///////////////////////////////////////////////////////
    PythonController(const size_t led_count_);
    PythonController(const PythonController &p) = default;

public: // public methods ////////////////////////////////////////////////////
    void update_percent(const double percent, const long duration_ms = 500);

private: // private members //////////////////////////////////////////////////
    size_t led_count;
    double current_percent;
    serial::SerialConnection serial;
};
