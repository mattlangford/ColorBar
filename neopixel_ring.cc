#include <iostream>
#include "neopixel_ring.hh"

//
// ############################################################################
//

NeopixelComms::NeopixelComms(const serial::SerialConnection &s)
    : CommunicationBase(s)
{
};

//
// ############################################################################
//

void NeopixelComms::send_frame(const animations::Frame &f)
{
    std::cout << "sending frame " << f.hold_s << std::endl;
};

//
// ############################################################################
//

enum bits : unsigned char
{
    ZERO = 0xC0,
    ONE = 0xF8
};

int main()
{
    serial::SerialConnection s;
    NeopixelComms c(s);
}
