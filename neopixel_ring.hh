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

public: // methods ////////////////////////////////////////////////////////////
    //
    // Send a frame to the neopixel display one byte at a time
    //
    void send_frame(const animations::Frame &f);

};

