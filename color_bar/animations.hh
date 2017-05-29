#pragma once
#include <memory>
#include <stddef.h>
#include <vector>

namespace serial
{
    class SerialConnection;
}

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
static Color BLUE = Color(0, 0, 20);
static Color GREEN = Color(0, 20, 0);
static Color RED = Color(20, 0, 0);
static Color WHITE = Color(255, 255, 255);

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

//
// Base class that provides general interface for displaying Frames on a display
// Maybe this shouldn't go here?
//
class CommunicationBase
{
public: // constructor ////////////////////////////////////////////////////////
    //
    //
    //
    CommunicationBase() = default;

    //
    //
    //
    virtual ~CommunicationBase() = default;

public: // methods ///////////////////////////////////////////////////////////
    //
    // Given a frame, return a ByteVector_t to send over the wire
    //
    virtual std::vector<unsigned char> build_frame(const animations::Frame &f) = 0;
};
using CommunicationBase_ptr = std::shared_ptr<CommunicationBase>;

//
// Functions that help build animations
//

//
// Play a vector of frames using the communication device and serial connection
//
void play_frames(const std::vector<Frame> &frames,
                 const std::shared_ptr<CommunicationBase> comms,
                 const serial::SerialConnection &serial);

//
// Builds a frame with some percent of the entire frame being green and the rest
// being red
//
Frame green_percent_bar(const double percent, const size_t led_count);

//
// Builds a vector of frames that transitions between two percentages
// in some number of steps
//
std::vector<Frame> green_percent_bar_ramp(const double percent_start,
                                          const double percent_end,
                                          const size_t led_count,
                                          const unsigned long duration_ms,
                                          const size_t step_count = 100);

std::vector<Frame> fade(const Frame &frame_start,
                        const Frame &frame_end,
                        const double duration_ms,
                        const size_t step_count = 100);

} // namespace colorama
