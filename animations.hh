#pragma once
#include <chrono>
#include <stddef.h>
#include <vector>

namespace animations
{

using uchar_t = unsigned char;

struct Color
{
    uchar_t R;
    uchar_t G;
    uchar_t B;
    uchar_t A;

    //
    // Default is black
    //
    Color() :
        R(0), G(0), B(0), A(255)
    {
    }

    Color(const uchar_t R_, const uchar_t G_, const uchar_t B_) :
        R(R_), G(G_), B(B_), A(255)
    {
    }

    Color(const uchar_t R_, const uchar_t G_, const uchar_t B_, const uchar_t A_) :
        R(R_), G(G_), B(B_), A(A_)
    {
    }
};

static Color BLACK = Color(0, 0, 0);
static Color BLUE = Color(0, 0, 255);
static Color RED = Color(255, 0, 0);

//
// Single Frame struct
// A Frame consists of a state of all `count` LED's, the frame contains
// some extra information too about how to get to the frame from the
// previous frame
//
struct Frame
{
    //
    // Matches LED numbers one to one. Index 0 represents the first LED
    //
    std::vector<Color> colors;

    //
    // How long (in milliseconds) should we hold on this frame before going to the next
    //
    unsigned long hold_time_ms;
};

class FramePlayer
{
//
// Playback a series of frames given a communication device
//



};

} // namespace colorama
