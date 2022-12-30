//
// Created by Lenart on 12/11/2022.
//

#ifndef ASS2_MEMORY_H
#define ASS2_MEMORY_H


#include <cstdint>
#include <array>
#include <memory>
#include <ostream>
#include "../common/SicTypes.h"

struct MemoryChange {
    Address_t start_address {};
    uint8_t changed_bytes_length {};
    uint32_t previous_value {};
    uint32_t new_value {};

    friend std::ostream &operator<<(std::ostream &os, const MemoryChange &change);

    static MemoryChange invalid() {
        return {
            .start_address = 0,
            .changed_bytes_length = 0,
            .previous_value = {},
            .new_value = {}
        };
    }
};

class Memory final {
public:
    Memory();
    Memory(Memory&& other) noexcept;
    ~Memory() = default;
public:
    [[nodiscard]] Byte_t get_byte(Address_t addr) const;
    MemoryChange set_byte(Address_t addr, Byte_t b);

    [[nodiscard]] Word_t get_word(Address_t addr) const;
    MemoryChange set_word(Address_t addr, Word_t b);

    [[nodiscard]] Float_t get_float(Address_t addr) const;
    MemoryChange set_float(Address_t addr, Float_t b);

    void undo(MemoryChange change);

    static constexpr int mem_size = 1<<20;
private:
    std::unique_ptr<std::array<uint8_t, mem_size>> m_alloc {};
};

#endif //ASS2_MEMORY_H
