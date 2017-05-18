#include <iostream>
#include <thread>
#include "neopixel_ring.hh"

//
// ### constructor ############################################################
//

NeopixelComms::NeopixelComms(const serial::SerialConnection &s)
    : CommunicationBase(s)
{
};

//
// ### public methods #########################################################
//

void NeopixelComms::send_frame(const animations::Frame &f)
{
    //
    // Reserve our frame buffer - how many bytes we will need
    // to command some number of LED's. Multiply that by 8 since it
    // takes us one byte to transmit one bit
    //
    ByteVector_t frame_buffer;
    frame_buffer.reserve(f.colors.size() * 3 * 8);

    for (const animations::Color color : f.colors)
    {
        //
        // To set a color, send it's GRB color, each component should be
        // sent MSB first.
        //
        for (const BYTE green_bit : convert_byte_to_spi(color.G))
            frame_buffer.push_back(green_bit);
        for (const BYTE red_bit : convert_byte_to_spi(color.R))
            frame_buffer.push_back(red_bit);
        for (const BYTE blue_bit : convert_byte_to_spi(color.B))
            frame_buffer.push_back(blue_bit);
    }

    //
    // We have built this frame buffer, now send it and wait for however
    // long this frame should stay displayed
    //
    get_serial().spi_write_data(frame_buffer);
    std::chrono::milliseconds duration(f.hold_time_ms);
    std::this_thread::sleep_for(duration);
};

//
// ### private methods ########################################################
//

NeopixelComms::ByteVector_t NeopixelComms::convert_byte_to_spi(const BYTE &byte)
{
    //
    // We will return a ByteVector composed of 8 bytes. The MSB of the original
    // byte will be the zeroth index in the vector
    //
    ByteVector_t bytes(8);
    BYTE mask = 0b10000000;
    for (size_t i = 0; i < 8; ++i)
    {
        bytes[i] = static_cast<int>(byte && mask) == 0 ? ZERO : ONE;
        mask = mask >> 1;
    }

    return bytes;
}

//
// ############################################################################
//

int main()
{
    serial::SerialConnection s;
    s.configure_spi_defaults();

    s.write_data({0x8F, 0xFF, 0xFF});

    // NeopixelComms c(s);

    // animations::Frame frame;
    // frame.colors = std::vector<animations::Color>(24, animations::BLUE);
    // frame.hold_time_ms = 500;

    // while (true)
    //     c.send_frame(frame);
}
