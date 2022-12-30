//
// Created by Lenart on 05/12/2022.
//

#include <sstream>
#include <iomanip>
#include "Disassembler.h"
#include "../common/Mnemonics.h"
#include "../common/Flags.h"

Disassembler::Disassembler(std::shared_ptr<Memory> memory) : m_memory(std::move(memory)) {}

std::string Disassembler::dissasemble_at(Address_t address) {
    auto b0 = m_memory->get_byte(address++);

    auto mnemonic = get_instruction_mnemonic(static_cast<Opcode>(b0 & 0b11111100));

    auto ss = std::stringstream {};

    if (!mnemonic) {
        ss << "Invalid byte [0x" << std::setfill('0') << std::setw(2) << std::hex << b0 << "]";
        return ss.str();
    }

    switch (mnemonic->format) {
        case Format::F3:
        case Format::F1: {
            ss << mnemonic->mnemonic;
            return ss.str();
        }
        case Format::F2_num:
        case Format::F2_reg:
        case Format::F2_reg_num:
        case Format::F2_reg_reg: {
            auto b1 = m_memory->get_byte(address++);
            auto num_1 = (b1 & 0xF0) >> 4;
            auto num_2 = (b1 & 0x0F);
            auto reg_1 = static_cast<Register>(num_1);
            auto reg_2 = static_cast<Register>(num_2);

            ss << mnemonic->mnemonic << " ";

            auto f = mnemonic->format;
            if (f == Format::F2_num) {
                ss << num_1;
            } else {
                ss << register_to_str(reg_1);
            }

            if (f == Format::F2_reg) return ss.str();

            ss << ", ";

            if (f == Format::F2_reg_num) {
                ss << num_2;
            } else {
                ss << register_to_str(reg_2);
            }

            return ss.str();
        }
        case Format::F3_4_mem: {
            auto b1 = m_memory->get_byte(address++);
            auto b2 = m_memory->get_byte(address++);

            Flags flags{b0, static_cast<uint8_t>(b1 >> 4)};

            ss << mnemonic->mnemonic << " ";

            if (!flags.is_valid()) {
                ss << "invalid flag configuration";
                return ss.str();
            }

            int32_t operand {};

            std::string_view operand_prefix {};
            std::string_view operand_relative {};
            std::string_view operand_postfix {};

            // Standard SIC
            if (flags.is_sic()) {
                operand = ((b1 & 0x7F) << 8) | b2;
            }
                // F4
            else if (flags.is_extended()) {
                uint8_t b3 = m_memory->get_byte(address++);
                operand = (b1 & 0x0F) << 16 | b2 << 8 | b3;

                std::stringstream temp;
                temp << "+";
                temp << ss.rdbuf();
                ss.swap(temp);
            }
                // F3
            else {
                operand = (b1 & 0x0F) << 8 | b2;

                if (operand & 0x800000) operand |= 0xff000000; // NOLINT(cppcoreguidelines-narrowing-conversions)
                else operand &= ~0xff000000; // NOLINT(cppcoreguidelines-narrowing-conversions)

                if (flags.is_immediate()) {
                    operand_prefix = "#";
                } else if(flags.is_indirect()) {
                    operand_prefix = "@";
                }

                if (flags.is_pc_relative()) {
                    operand = operand >= 2048 ? operand - 4096 : operand;
                    if (operand >= 0) {
                        operand_relative = "(PC) + ";
                    } else {
                        if (mnemonic->opcode == Opcode::J && operand == -3)
                            operand_postfix = " [halt]";
                        operand_relative = "(PC) - ";
                        operand = -operand;
                    }
                } else if (flags.is_base_relative()) {
                    operand_relative = "(B) + ";
                }
            }

            ss << operand_prefix << operand_relative << "0x" << std::hex << operand << operand_postfix;

            if (flags.is_indexed()) {
                ss << ", X";
            }

            return ss.str();
        }
        default:
            break;
    }

    assert(!"Unknown format");
    return {};
}
