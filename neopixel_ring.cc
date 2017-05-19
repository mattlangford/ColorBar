#include <boost/python.hpp>
#include <iostream>
#include <thread>
#include "neopixel_ring.hh"

//
// ### public methods #########################################################
//

NeopixelComms::ByteVector_t NeopixelComms::build_frame(const animations::Frame &f)
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
    return std::move(frame_buffer);
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
        bytes[i] = static_cast<int>(byte & mask) == 0 ? ZERO : ONE;
        mask = mask >> 1;
    }

    return bytes;
}

//
// ############################################################################
//


// BOOST_PYTHON_MODULE(neopixel_driver)
// {
//     using namespace boost::python;
//     def("test", test);
// }

int main()
{
    serial::SerialConnection s;
    s.configure_spi_defaults();

    serial::CommunicationBase_ptr neopixel = std::make_unique<NeopixelComms>();

    std::vector<animations::Frame> frames = animations::green_percent_bar_ramp(0, 1, 24, 500, 24);

    animations::play_frames(frames, neopixel, s);

    std::chrono::seconds sleep(1);
    std::this_thread::sleep_for(sleep);

    animations::Frame bg = frames.back();
    bg.hold_time_ms = 50;
    animations::Frame blue;
    blue.colors = std::vector<animations::Color>(24, animations::BLUE);
    blue.hold_time_ms = 50;

    frames = {blue, bg, blue, bg, blue, bg};
    animations::play_frames(frames, neopixel, s);

    frames = animations::green_percent_bar_ramp(1, 0.6, 24, 500);
    animations::play_frames(frames, neopixel, s);
}
