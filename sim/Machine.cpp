//
// Created by Lenart on 12/11/2022.
//

#include <cassert>
#include <iostream>
#include <memory>
#include <utility>
#include "Machine.h"
#include "../common/Flags.h"

Machine::Machine(Address_t start_address, std::shared_ptr<Memory> memory)
    : m_memory(std::move(memory))
{
    m_registers.setPc(start_address);
    m_devices[0] = std::make_unique<StdinDevice>();
    m_devices[1] = std::make_unique<StdoutDevice>();
    m_devices[2] = std::make_unique<StderrDevice>();
}

void Machine::step() {
    execute();
}

void Machine::undo() {
    while (!m_changes.empty()) {
        auto change = m_changes.back();
        m_changes.pop_back();

        if (std::holds_alternative<RegisterChange>(change)) {
            m_registers.undo(std::get<RegisterChange>(change));
        }
        else if (std::holds_alternative<MemoryChange>(change)) {
            m_memory->undo(std::get<MemoryChange>(change));
        }
        else if (std::holds_alternative<ChangeStart>(change)) {
            m_halted = false;
            break;
        }
        else {
            assert(!"Unknown change variant");
        }
    }
}

void Machine::execute() {
    if (m_halted) return;

    add_change_step(ChangeStart{m_registers.getPc()});

    uint8_t b0 = fetch();

    if (execute_format1(b0)) return;

    uint8_t b1 = fetch();
    if (execute_format2(b0, b1)) return;

    uint8_t b2 = fetch();
    if (execute_format3_4(b0, b1, b2)) return;

    assert(!"Can't parse instruction");
}

bool Machine::execute_format1(uint8_t b0) {
    auto op = static_cast<Opcode>(b0 & 0b11111100);
    switch (op) {
        case Opcode::FLOAT:
        case Opcode::FIX:
        case Opcode::NORM:
        case Opcode::SIO:
        case Opcode::HIO:
        case Opcode::TIO:
            not_implemented(op);
            break;
        default:
            return false;
    }
    return true;
}

void Machine::set_register(Register reg, Register_t new_value) {
    add_change_step(m_registers.set(reg, new_value));
}

void Machine::set_word(const Flags &flags, Address_t address, Word_t new_value) {
    address = resolve_address(flags, address);
    add_change_step(m_memory->set_word(address, new_value));
}

Word_t Machine::get_word(const Flags &flags, Address_t address) {
    if (flags.is_immediate()) return static_cast<Word_t>(address);
    address = resolve_address(flags, address);
    return m_memory->get_word(address);
}

void Machine::set_byte(const Flags &flags, Address_t address, Byte_t new_value) {
    address = resolve_address(flags, address);
    add_change_step(m_memory->set_byte(address, new_value));
}

Byte_t Machine::get_byte(const Flags &flags, Address_t address) {
    if (flags.is_immediate()) return static_cast<Byte_t>(address);
    address = resolve_address(flags, address);
    return m_memory->get_byte(address);
}


Address_t Machine::resolve_address(const Flags &flags, Address_t address) {
    if (flags.is_indirect())
        return m_memory->get_word(address);
    return address;
}


bool Machine::execute_format2(uint8_t b0, uint8_t b1) {
    auto op = static_cast<Opcode>(b0 & 0b11111100);
    auto num_1 = (b1 & 0xF0) >> 4;
    auto num_2 = (b1 & 0x0F);
    auto reg_1 = static_cast<Register>(num_1);
    auto reg_2 = static_cast<Register>(num_2);

    switch (op) {
        case Opcode::ADDR:
            set_register(reg_2, m_registers.get(reg_1) + m_registers.get(reg_2));
            break;
        case Opcode::SUBR:
            set_register(reg_2, m_registers.get(reg_1) - m_registers.get(reg_2));
            break;
        case Opcode::MULR:
            set_register(reg_2, m_registers.get(reg_1) * m_registers.get(reg_2));
            break;
        case Opcode::DIVR:
            set_register(reg_2, m_registers.get(reg_1) / m_registers.get(reg_2));
            break;
        case Opcode::COMPR:
            set_register(Register::CC, m_registers.get(reg_1) - m_registers.get(reg_2));
            break;
        case Opcode::SHIFTL: {
            auto reg = m_registers.get(reg_1);
            auto rotated_left = (reg << (num_2 + 1) | ((reg & 0x800000) > 0));
            set_register(reg_1, rotated_left);
        }
        case Opcode::SHIFTR: {
            auto reg = m_registers.get(reg_1);
            auto rotated_right = (reg >> (num_2 + 1) | ((reg & 1) << 23));
            set_register(reg_1, rotated_right);
        }
        case Opcode::RMO:
            set_register(reg_2, m_registers.get(reg_1));
            break;
        case Opcode::CLEAR:
            set_register(reg_1, {});
            break;
        case Opcode::TIXR:
            set_register(Register::X, m_registers.getX() + 1);
            set_register(Register::CC, m_registers.getX() - m_registers.get(reg_1));
            break;
        case Opcode::SVC:
            not_implemented(Opcode::SVC);
            break;
        default:
            return false;
    }
    return true;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
bool Machine::execute_format3_4(uint8_t b0, uint8_t b1, uint8_t b2) {
    auto op = static_cast<Opcode>(b0 & 0b11111100);

    Flags flags{b0, static_cast<uint8_t>(b1 >> 4)};

    if (!flags.is_valid()) return false;

    int32_t operand;

    // Standard SIC
    if (flags.is_sic()) {
        operand = ((b1 & 0x7F) << 8) | b2;
    }
        // F4
    else if (flags.is_extended()) {
        uint8_t b3 = fetch();
        operand = (b1 & 0x0F) << 16 | b2 << 8 | b3;
    }
        // F3
    else {
        operand = (b1 & 0x0F) << 8 | b2;

        if (flags.is_pc_relative()) {
            operand = operand >= 2048 ? operand - 4096 : operand;
            operand += m_registers.getPc();
        } else if (flags.is_base_relative()) {
            operand += m_registers.getB();
        }
    }

    if (flags.is_indexed()) {
        operand += m_registers.getX();
    }

    switch (op) {
        case Opcode::STA:
            set_word(flags, operand, m_registers.getA());
            break;
        case Opcode::STX:
            set_word(flags, operand, m_registers.getX());
            break;
        case Opcode::STL:
            set_word(flags, operand, m_registers.getL());
            break;
        case Opcode::STCH:
            set_byte(flags, operand, m_registers.getA());
            break;
        case Opcode::STB:
            set_word(flags, operand, m_registers.getB());
            break;
        case Opcode::STS:
            set_word(flags, operand, m_registers.getS());
            break;
        case Opcode::STT:
            set_word(flags, operand, m_registers.getT());
            break;
        case Opcode::STSW:
            set_word(flags, operand, m_registers.getSw());
            break;
        case Opcode::JEQ:
            if (m_registers.CC_is_equal()) set_register(Register::PC, resolve_address(flags, operand));
            break;
        case Opcode::JGT:
            if (m_registers.CC_is_greater()) set_register(Register::PC, resolve_address(flags, operand));
            break;
        case Opcode::JLT:
            if (m_registers.CC_is_lower()) set_register(Register::PC, resolve_address(flags, operand));
            break;
        case Opcode::J: {
            auto new_address= resolve_address(flags, operand);
            m_halted = new_address == m_registers.getPc() - 3;

            set_register(Register::PC, new_address);
            break;
        }
        case Opcode::RSUB:
            set_register(Register::PC, m_registers.getL());
            break;
        case Opcode::JSUB:
            set_register(Register::L, m_registers.getPc());
            set_register(Register::PC, resolve_address(flags, operand));
            break;
        case Opcode::LDA:
            set_register(Register::A, get_word(flags, operand));
            break;
        case Opcode::LDX:
            set_register(Register::X, get_word(flags, operand));
            break;
        case Opcode::LDL:
            set_register(Register::L, get_word(flags, operand));
            break;
        case Opcode::LDCH: {
            auto reg_A = m_registers.getA();
            auto ch = get_byte(flags, operand);
            reg_A = reg_A & 0xffff00 | ch;
            set_register(Register::A, reg_A);
            break;
        }
        case Opcode::LDB:
            set_register(Register::B, get_word(flags, operand));
            break;
        case Opcode::LDS:
            set_register(Register::S, get_word(flags, operand));
            break;
        case Opcode::LDT:
            set_register(Register::T, get_word(flags, operand));
            break;
        case Opcode::ADD:
            set_register(Register::A, m_registers.getA() + get_word(flags, operand));
            break;
        case Opcode::SUB:
            set_register(Register::A, m_registers.getA() - get_word(flags, operand));
            break;
        case Opcode::MUL:
            set_register(Register::A, m_registers.getA() * get_word(flags, operand));
            break;
        case Opcode::DIV:
            set_register(Register::A, m_registers.getA() / get_word(flags, operand));
            break;
        case Opcode::AND:
            set_register(Register::A, m_registers.getA() & get_word(flags, operand));
            break;
        case Opcode::OR:
            set_register(Register::A, m_registers.getA() | get_word(flags, operand));
            break;
        case Opcode::COMP:
            set_register(Register::CC,m_registers.getA() - get_word(flags, operand));
            break;
        case Opcode::TIX:
            set_register(Register::X, m_registers.getX() + 1);
            set_register(Register::CC, m_registers.getX() - get_word(flags, operand));
            break;
        case Opcode::RD: {
            auto device_id = get_byte(flags, operand);

            if (!m_devices.contains(device_id)) {
                m_devices[device_id] = std::make_unique<FileDevice>(device_id, false);
            }

            auto& device = m_devices[device_id];

            auto reg_A = m_registers.getA();
            auto ch = device->read();
            reg_A = reg_A & 0xffff00 | ch;
            set_register(Register::A, reg_A);
            break;
        }
        case Opcode::WD: {
            auto device_id = get_byte(flags, operand);
            if (!m_devices.contains(device_id)) {
                m_devices[device_id] = std::make_unique<FileDevice>(device_id, true);
            }

            auto& device = m_devices[device_id];

            auto reg_A = m_registers.getA();
            device->write(reg_A & 0xff);
            std::cout << std::flush;
            break;
        }
        case Opcode::TD: {
            auto device_id = get_byte(flags, operand);

            bool tested = false;
            if (m_devices.contains(device_id)) {
                auto& device = m_devices[device_id];
                tested = device->test();
            }

            set_register(Register::CC, tested ? 0 : -1);
            break;
        }
        case Opcode::LDF:
        case Opcode::STF:
        case Opcode::ADDF:
        case Opcode::SUBF:
        case Opcode::MULF:
        case Opcode::DIVF:
        case Opcode::COMPF:
        case Opcode::LPS:
        case Opcode::STI:
        case Opcode::SSK:
            not_implemented(op);
            break;
        default:
            return false;
    }

    return true;
}
#pragma clang diagnostic pop


uint8_t Machine::fetch() {
    auto pc = m_registers.getPc();
    add_change_step(m_registers.setPc(pc + 1));
    return m_memory->get_byte(pc);
}

void Machine::not_implemented(Opcode opcode) {
    auto instruction = get_instruction_mnemonic(opcode);

    std::cout << "Machine: Instruction not implemented: " <<
              (instruction ? instruction->mnemonic : "") << std::endl;
    assert(!"Instruction not implemented");
}

void Machine::add_change_step(Machine::Change_t change) {
    if (!m_changes.empty() && std::holds_alternative<RegisterChange>(m_changes.back()) && std::holds_alternative<RegisterChange>(change) ) {
        auto& register_change = std::get<RegisterChange>(change);
        auto& last_register_change = std::get<RegisterChange>(m_changes.back());

        if (register_change.register_id == last_register_change.register_id) {
            last_register_change.new_value = register_change.new_value;
            return;
        }
    }

    m_changes.push_back(change);
    if (m_changes.size() >= max_changes) {
        do {
            m_changes.pop_front();
        } while (!m_changes.empty() && !std::holds_alternative<ChangeStart>(m_changes.at(0)));
    }
}

bool Machine::in_halt_condition() const {
    return m_halted;
}

const Registers &Machine::get_registers() const {
    return m_registers;
}

std::shared_ptr<Memory> Machine::get_memory() const {
    return m_memory;
}

bool Machine::can_undo() {
    return !m_changes.empty();
}

void Machine::set_execution_breakpoint(Address_t breakpoint_address) {
    m_execution_breakpoints.insert(breakpoint_address);
}

void Machine::clear_execution_breakpoint(Address_t breakpoint_address) {
    m_execution_breakpoints.erase(breakpoint_address);
}

void Machine::clear_execution_breakpoints() {
    m_execution_breakpoints.clear();
}

void Machine::run() {
    while (!pc_is_on_breakpoint() && !in_halt_condition()) {
        step();
    }
}

bool Machine::pc_is_on_breakpoint() {
    return m_execution_breakpoints.contains(m_registers.getPc());
}

const std::deque<Machine::Change_t>& Machine::get_changes() {
    return m_changes;
}
