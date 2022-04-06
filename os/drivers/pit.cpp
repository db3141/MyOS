#include "common.hpp"
#include "pit.hpp"
#include "interrupts/pic.hpp"

namespace Kernel::PIT {

    enum Port : u8 {
        PORT_CHANNEL_0 = 0x40,
        PORT_CHANNEL_1 = 0x41,
        PORT_CHANNEL_2 = 0x42,
        PORT_COMMAND_REGISTER = 0x43
    };

    constexpr Port get_port(Channel t_channel) {
        switch (t_channel) {
            case Channel::ZERO:
                return PORT_CHANNEL_0;
            case Channel::ONE:
                return PORT_CHANNEL_1;
            case Channel::TWO:
                return PORT_CHANNEL_2;
        }
    }

    constexpr u32 round_div(u32 a, u32 b) {
        const u32 t = a / b;
        const u32 d1 = a - (t * b);
        const u32 d2 = ((t + 1) * b) - a;
        return (d1 < d2) ? (t) : (t + 1);
    }

    constexpr u32 BASE_FREQUENCY = 1193182;

    static u32 ticks;

    void initialize() {
        ticks = 0;
        set_frequency(Channel::ZERO, 1000);
    }

    void set_frequency(Channel t_channel, u32 t_frequency) {
        if (t_frequency == 0) {
            return; // TODO: error codes
        }

        const u16 count = round_div(BASE_FREQUENCY, t_frequency) & ~1; // masked with ~1 since we dont want it to be odd

        const u8 channelBits = static_cast<u8>(t_channel) << 6;
        const u8 accessBits = 0b0011'0000; // lobyte/hibyte
        const u8 operatingModeBits = 0b0000'0110; // mode 3 - square wave generator
        const u8 BCDBit = 0b0000'0000; // 16-bit binary
        const u8 command = channelBits | accessBits | operatingModeBits | BCDBit;

        port_write_byte(PORT_COMMAND_REGISTER, command);
        io_wait();
        port_write_byte(get_port(t_channel), count & 0x00FF);
        io_wait();
        port_write_byte(get_port(t_channel), count >> 8);
        io_wait();
    }

    u32 get_ticks() {
        return ticks;
    }

    INTERRUPT_HANDLER void interval_handler(InterruptHandler::InterruptFrame* t_frame) {
        ticks++;
        PIC::send_end_of_interrupt(0x20);
    }

}
