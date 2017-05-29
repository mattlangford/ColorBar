#include <assert.h>
#include <chrono>
#include <iostream>
#include <thread>

#include "animations.hh"
#include "../ftd2xx_driver/serial.hh"

namespace animations
{

void play_frames(const std::vector<Frame> &frames,
                 const CommunicationBase_ptr comms,
                 const serial::SerialConnection &serial)
{
    for (const Frame &frame : frames)
    {
        serial.spi_write_data(comms->build_frame(frame));

        std::chrono::milliseconds duration(frame.hold_time_ms);
        std::this_thread::sleep_for(duration);
    }
}

Frame green_percent_bar(const double percent, const size_t led_count)
{
    assert(percent <= 1.0);
    const size_t green_pixels = static_cast<size_t>(led_count * percent);

    Frame f;
    f.colors = std::vector<Color>(led_count, RED);
    for (size_t i = 0; i < green_pixels; ++i)
    {
        f.colors[i] = GREEN;
    }

    return f;
}

std::vector<Frame> green_percent_bar_ramp(const double percent_start,
                                          const double percent_end,
                                          const size_t led_count,
                                          const unsigned long duration_ms,
                                          const size_t step_count)
{
    std::vector<animations::Frame> frames;
    const double step = (percent_end - percent_start) / step_count;
    double p = percent_start;
    for (size_t i = 0; i < step_count; ++i)
    {
        animations::Frame this_percent = green_percent_bar(p, led_count);
        this_percent.hold_time_ms = duration_ms / step_count;
        frames.push_back(this_percent);
        p += step;
    }

    animations::Frame last_percent = green_percent_bar(percent_end, led_count);
    last_percent.hold_time_ms = duration_ms / step_count;
    frames.push_back(last_percent);

    return frames;
}

std::vector<Frame> fade(const Frame &frame_start,
                        const Frame &frame_end,
                        const double duration_ms,
                        const size_t step_count)
{
    // TODO
    return {};
}
} // namespace animations
