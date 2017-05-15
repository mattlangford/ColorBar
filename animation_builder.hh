namespace colorama
{


struct Color
{
    unsigned char R;
    unsigned char G;
    unsigned char B;

    Color(const unsigned char R_, const unsigned char G_, const unsigned char B_) :
        R(R_), G(G_), B(B_)
    {
    }
};

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
    // Matches LED numbers one to one. Index 0 represents the first LED.
    //
    Color colors[led_count];

    //
    // How long (in seconds) it will take to fade between the previous
    // frame and this frame.
    //
    double prefade_s = 0.0;

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
