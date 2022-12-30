//
// Created by Lenart on 12/11/2022.
//

#ifndef ASS2_MACHINE_H
#define ASS2_MACHINE_H

#include <memory>
#include <variant>
#include <deque>
#include <set>
#include "Memory.h"
#include "Registers.h"
#include "../common/Mnemonics.h"
#include "../common/Flags.h"
#include "Device.h"

class Machine {
public:
    Machine(Address_t start_address, std::shared_ptr<Memory> memory);

    // Machine control
    void step();
    void run();
    bool can_undo();
    void undo();

    void set_execution_breakpoint(Address_t breakpoint_address);
    void clear_execution_breakpoint(Address_t breakpoint_address);
    void clear_execution_breakpoints();

    [[nodiscard]] const Registers &get_registers() const;
    [[nodiscard]] std::shared_ptr<Memory> get_memory() const;
    struct ChangeStart { Address_t pc; };
    using Change_t = std::variant<ChangeStart, MemoryChange, RegisterChange>;
    [[nodiscard]] const std::deque<Change_t>& get_changes();
    [[nodiscard]] bool in_halt_condition() const;

private:
    [[nodiscard]] uint8_t fetch();

    void execute();

    [[nodiscard]] bool execute_format1(uint8_t b0);
    [[nodiscard]] bool execute_format2(uint8_t b0, uint8_t b1);
    [[nodiscard]] bool execute_format3_4(uint8_t b0, uint8_t b1, uint8_t b2);

    void set_register(Register reg, Register_t new_value);

    [[nodiscard]] Address_t resolve_address(const Flags& flags, Address_t address);

    void set_word(const Flags& flags, Address_t address, Word_t new_value);
    void set_byte(const Flags& flags, Address_t address, Byte_t new_value);
    [[nodiscard]] Word_t get_word(const Flags& flags, Address_t address);
    [[nodiscard]] Byte_t get_byte(const Flags& flags, Address_t address);

    void add_change_step(Change_t change);

    bool pc_is_on_breakpoint();

    static void not_implemented(Opcode opcode);
private:
    std::shared_ptr<Memory> m_memory;
    Registers m_registers {};

    std::map<Byte_t, std::unique_ptr<Device>> m_devices;

    static constexpr size_t max_changes = 200;
    std::deque<Change_t> m_changes {};

    std::set<Address_t> m_execution_breakpoints {};

    bool m_halted { false };
};


#endif //ASS2_MACHINE_H
