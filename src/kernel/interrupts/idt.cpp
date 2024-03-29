#include "idt.hpp"
#include "interrupt_handler.hpp"

namespace Kernel::IDT {

    struct IDTEntry {
        u16 offsetLo;
        u16 segmentSelector;
        u8 _; // reserved
        u8 misc;
        u16 offsetHi;
    };

    struct IDT {
        u16 _; // unused
        u16 size;
        IDTEntry* entries;
    };

    IDTEntry idt_entry_create(u32 t_offset, u16 t_segmentSelector, IDTGateType t_gateType, bool t_32bit);

    constexpr u8 generate_misc_byte(IDTGateType t_gateType, bool t_32bit) {
        switch (t_gateType) {
            case IDTGateType::TASK:
                return 0b10000101;
            case IDTGateType::INTERRUPT:
                if (!t_32bit) {
                    return 0b10000110; // 16-bit
                }
                else {
                    return 0b10001110; // 32-bit
                }
            case IDTGateType::TRAP:
                if (!t_32bit) {
                    return 0b10000111; // 16-bit
                }
                else {
                    return 0b10001111; // 32-bit
                }
        }
    }

    constexpr size_t IDT_MAX_ENTRY_COUNT = 256;

    static IDTEntry IDT_ENTRIES[IDT_MAX_ENTRY_COUNT];
    static IDT KERNEL_IDT = { 0, 0, nullptr };

    void initialize() {
        KERNEL_IDT.entries = IDT_ENTRIES;
        for (size_t i = 0; i < IDT_MAX_ENTRY_COUNT; i++) {
            set_entry(i, (void*)&InterruptHandler::interrupt_handler, 0x00000008, IDTGateType::INTERRUPT, true);
        }
        KERNEL_IDT.size = sizeof(IDTEntry) * IDT_MAX_ENTRY_COUNT;
    }

    IDTEntry idt_entry_create(u32 t_offset, u16 t_segmentSelector, IDTGateType t_gateType, bool t_32bit) {
        IDTEntry result = { 0, 0, 0, 0, 0 };

        if (t_gateType != IDTGateType::TASK) {
            result.offsetHi = static_cast<u16>((t_offset >> 16) & 0xFFFFFFFF);
            result.offsetLo = static_cast<u16>((t_offset >>  0) & 0xFFFFFFFF);
        }
        result.misc = generate_misc_byte(t_gateType, t_32bit);
        result.segmentSelector = t_segmentSelector;

        return result;
    }

    Data::ErrorOr<void> set_entry(size_t t_index, void* t_handlerAddress, u16 t_segmentSelector, IDTGateType t_gateType, bool t_32bit) {
        if (t_index >= IDT_MAX_ENTRY_COUNT) {
            return Error::INDEX_OUT_OF_RANGE;
        }

        const IDTEntry entry = idt_entry_create(reinterpret_cast<u32>(t_handlerAddress), t_segmentSelector, t_gateType, t_32bit);
        KERNEL_IDT.entries[t_index] = entry;

        return Data::ErrorOr<void>();
    }

    void load_table() {
        KERNEL_IDT.size -= 1; // size should contain size of all entries - 1

        u8* idtPointer = ((u8*) &KERNEL_IDT) + 2; // ignore unused bytes in struct
        asm volatile ("lidt (%0)" : : "r" (idtPointer));
    }

}
