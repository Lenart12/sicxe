//
// Created by Lenart on 12/11/2022.
//

#include <cassert>
#include <iomanip>
#include "Memory.h"

Memory::Memory()
    : m_alloc(new decltype(m_alloc)::element_type {})
{
//    std::fill(m_alloc->begin(), m_alloc->end(), 0);
}

Memory::Memory(Memory &&other) noexcept
    : m_alloc(std::move(other.m_alloc))
{}

Byte_t Memory::get_byte(Address_t addr) const {
    if (addr >= mem_size) return 0;
    return (*m_alloc)[addr];
}

MemoryChange Memory::set_byte(Address_t addr, Byte_t b) {
    if (addr >= mem_size) return MemoryChange::invalid();
    MemoryChange change {
            .start_address = addr,
            .changed_bytes_length = 1,
            .previous_value = get_byte(addr),
            .new_value = b
    };
    (*m_alloc)[addr] = b;
    return change;
}

Word_t Memory::get_word(Address_t addr) const {
    if (addr + 2 >= mem_size) return 0;
    auto word = (*m_alloc)[addr] << 16 | (*m_alloc)[addr + 1] << 8 | (*m_alloc)[addr + 2];
    if (word & 0x800000) word |= 0xff000000; // NOLINT(cppcoreguidelines-narrowing-conversions)
    else word &= ~0xff000000; // NOLINT(cppcoreguidelines-narrowing-conversions)
    return word;
}

MemoryChange Memory::set_word(Address_t addr, Word_t b) {
    if (addr + 2 >= mem_size) return MemoryChange::invalid();
    MemoryChange change {
            .start_address = addr,
            .changed_bytes_length = 3,
            .previous_value = get_word(addr),
            .new_value = b
    };
    (*m_alloc)[addr] = (b & 0xff0000) >> 16;
    (*m_alloc)[addr + 1] = (b & 0xff00) >> 8;
    (*m_alloc)[addr + 2] = b & 0xff;
    return change;
}

// TODO
Float_t Memory::get_float(Address_t addr) const {
    return 0;
}

// TODO
MemoryChange Memory::set_float(Address_t addr, Float_t b) {
    if (addr + 5 >= mem_size) return MemoryChange::invalid();
    return MemoryChange::invalid();
}

void Memory::undo(MemoryChange change) {
    switch (change.changed_bytes_length) {
        case 1:
            set_byte(change.start_address, change.previous_value);
            break;
        case 3:
            set_word(change.start_address, change.previous_value);
            break;
        // TODO:
        case 6: assert(!"Cant undo float");
        default:
            assert(!"Unknown memory change");
    }
}


std::ostream &operator<<(std::ostream &os, const MemoryChange &change) {
    os << "start_address: 0x";
    os << std::setfill('0') << std::setw(6) << std::hex << change.start_address;
    os << " changed_bytes_length: " << std::dec << (int)change.changed_bytes_length << " previous_value: 0x";
    os << std::setfill('0') << std::setw(6) << std::hex << (change.previous_value & 0xffffff);
    os << " new_value: 0x";
    os << std::setfill('0') << std::setw(6) << std::hex << (change.previous_value & 0xffffff);
    return os;
}
