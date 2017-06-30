#pragma once
#include "animations.hh"
#include "../ftd2xx_driver/serial.hh"

class NeopixelComms final : public animations::CommunicationBase
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
    serial::ByteVector_t build_frame(const animations::Frame &f);

private: // methods ///////////////////////////////////////////////////////////
    //
    // Take a byte in and return a funky SPI formatted ByteVector.
    // Since the Neopixel communicates in a weird protocol, this
    // conversion is required.
    //
    serial::ByteVector_t convert_byte_to_spi(const BYTE &byte);

};

class PythonController
{
public: // constructor ///////////////////////////////////////////////////////
    PythonController(const size_t led_count_, const size_t pixel_groups_);
    PythonController(const PythonController &p) = default;

public: // public methods ////////////////////////////////////////////////////
    void update_frame(const std::vector<uint8_t> &frame);

private: // private members //////////////////////////////////////////////////
    size_t led_count;
    serial::SerialConnection serial;
};
