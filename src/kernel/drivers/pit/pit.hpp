#ifndef PIT_INCLUDED
#define PIT_INCLUDED

#include "common.hpp"
#include "interrupts/interrupt_handler.hpp"

namespace Kernel::PIT {
    
    enum class Channel : u8 {
        ZERO = 0b00,
        ONE  = 0b01,
        TWO  = 0b10,
    };

    void initialize();

    void set_frequency(Channel t_channel, uint t_frequency);
    uint get_ticks();

    constexpr uint TICKS_PER_SECOND = 1000; // TODO: handle this better

    INTERRUPT_HANDLER void interval_handler(InterruptHandler::InterruptFrame* t_frame);

}

#endif
