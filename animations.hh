#include <stddef.h>

namespace colorama
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
template <unsigned int led_count>
struct Frame
{
    //
    // Matches LED numbers one to one. Index 0 represents the first LED
    //
    Color colors[led_count];

    //
    // How long (in seconds) should we hold on this frame before going to the
    // next
    //
    double hold_s = 0.0;

    //
    // Should we keep the previous frame where we don't have defined colors
    //
    bool use_previous = false;

};

template <unsigned int led_count>
class AnimationBuilder
{
public: // constructor / destructor ///////////////////////////////////////////
    AnimationBuilder(const Frame<led_count> frame_)
    {
        frame = frame_;
    }

public: // methods ////////////////////////////////////////////////////////////
    template <unsigned int led_number, unsigned char R, unsigned char G, unsigned char B>
    constexpr AnimationBuilder set_color() const
    {
        Frame<led_count> new_frame = frame;
        new_frame.colors[led_number] = Color(R, G, B);
        return AnimationBuilder(new_frame);
    }

    template <unsigned int led_number>
    constexpr AnimationBuilder set_color(Color&& color) const
    {
        Frame<led_count> new_frame = frame;
        new_frame.colors[led_number] = color;
        return AnimationBuilder(new_frame);
    }

    inline Frame<led_count> build() const
    {
        return frame;
    }

private: // members ///////////////////////////////////////////////////////////
    Frame<led_count> frame;
};

} // namespace colorama
