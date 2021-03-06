#include <boost/python.hpp>
#include <iostream>
#include <thread>

#include "neopixel_driver.hh"

//
// ### public methods #########################################################
//

serial::ByteVector_t NeopixelComms::build_frame(const animations::Frame &f)
{
    //
    // Reserve our frame buffer - how many bytes we will need
    // to command some number of LED's. Multiply that by 8 since it
    // takes us one byte to transmit one bit
    //
    serial::ByteVector_t frame_buffer;
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
}

//
// ### private methods ########################################################
//

serial::ByteVector_t NeopixelComms::convert_byte_to_spi(const BYTE &byte)
{
    //
    // We will return a ByteVector composed of 8 bytes. The MSB of the original
    // byte will be the zeroth index in the vector
    //
    serial::ByteVector_t bytes(8);
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

PythonController::PythonController(const size_t led_count_, const size_t pixel_groups_)
    : led_count(led_count_), serial()
{
    serial.configure_spi_defaults(5E6);

    animations::Frame blank;
    blank.colors = std::vector<animations::Color>(led_count, animations::RED);

    NeopixelComms comms;
    serial.spi_write_data(comms.build_frame(blank));
}

//
// ############################################################################
//

BOOST_PYTHON_MODULE(neopixel_driver)
{
    // This only lets someone animate a green/red bar for the performance meter
    using namespace boost::python;
    class_<animations::Color>("Color")
        .def(init<animations::uchar_t, animations::uchar_t, animations::uchar_t>())
        .def_readwrite("r", &animations::Color::R)
        .def_readwrite("g", &animations::Color::G)
        .def_readwrite("b", &animations::Color::B);

    class_<animations::Frame>("Frame")
        .def(init<const std::vector<animations::Color>&>())
        .def_readwrite("colors", &animations::Frame::colors);

    // class_<PythonController>("NeoPixelDriver", init<const size_t>())
    //     .def("update_pixel", &PythonController::update_percent)
    //     .def("update_pixel_group", &PythonController::update_percent);
}
