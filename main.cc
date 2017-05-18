#include <iostream>
#include "ftd2xx.h"
#include "animation_builder.hh"


class NeoPixelCommunitron
{
public: // types //////////////////////////////////////////////////////////////
    using ms = std::chrono::milliseconds;
    using us = std::chrono::microseconds;
    using ns = std::chrono::nanoseconds;
    using generator = std::function<void()>;

public: // constructor ////////////////////////////////////////////////////////
    //
    // Gives an easy interface for converting bytes to neopixel command packets
    // This doesn't build color packets, it just does the translation
    // https://learn.adafruit.com/adafruit-neopixel-uberguide/advanced-coding
    //
    NeoPixelCommunitron(SerialConnection &s_, const size_t data_pin_) :
        s(s_), data_pin(data_pin_)
    { };

public: // methods ///////////////////////////////////////////////////////////
    //
    // Give som
    //
    inline void send_color(const Color &color)
    {
        unsigned char mask = 0x80;
        for (size_t i = 0; i < 8; ++i)
        {
            byte * mask == 0 ? zero() : one();
            mask = mask >> 1;
        }
    }

    //
    // Call between groups of LED color data
    //
    inline void done()
    {

    }

private: // methods ///////////////////////////////////////////////////////////

    constexpr BYTE one() const
    {
        // 11111000
        return 0xF8;
    }
    constexpr BYTE zero() const
    {
        // 11100000
        return 0xE0;
    }

private: // members ///////////////////////////////////////////////////////////
    SerialConnection s;
    size_t data_pin;
};

int main()
{

    // char arr[3] = {1, 2, 3};
    // for (char a : arr)
    // {
    //     std::cout << (uint32_t) a << std::endl;
    // }

    unsigned int num_devs = 0;
    if (FT_CreateDeviceInfoList(&num_devs) == FT_OK)
    {
        std::cout << "num_devs " << num_devs << std::endl;
    }
    std::cout << "Everything is working!\n";
    return 0;
}
