//
// Created by Lenart on 20/11/2022.
//

#include "Mnemonics.h"
#include <map>

using std::map;

#define Sym(name) Opcode::name, Token::Type::name, #name
constexpr std::array instruction_table {
        // Format 1
        InstructionMnemonic {Sym(FIX), Format::F1},
        InstructionMnemonic {Sym(FLOAT), Format::F1},
        InstructionMnemonic {Sym(NORM), Format::F1},
        InstructionMnemonic {Sym(SIO), Format::F1},
        InstructionMnemonic {Sym(HIO), Format::F1},
        InstructionMnemonic {Sym(TIO), Format::F1},
        // Format 2
        InstructionMnemonic {Sym(SVC), Format::F2_num},
        InstructionMnemonic {Sym(CLEAR), Format::F2_reg},
        InstructionMnemonic {Sym(TIXR), Format::F2_reg},
        InstructionMnemonic {Sym(SHIFTL), Format::F2_reg_num},
        InstructionMnemonic {Sym(SHIFTR), Format::F2_reg_num},
        InstructionMnemonic {Sym(ADDR), Format::F2_reg_reg},
        InstructionMnemonic {Sym(SUBR), Format::F2_reg_reg},
        InstructionMnemonic {Sym(MULR), Format::F2_reg_reg},
        InstructionMnemonic {Sym(DIVR), Format::F2_reg_reg},
        InstructionMnemonic {Sym(COMPR), Format::F2_reg_reg},
        InstructionMnemonic {Sym(RMO), Format::F2_reg_reg},
        // Format 3/4
        // Load/store
        InstructionMnemonic {Sym(LDA), Format::F3_4_mem},
        InstructionMnemonic {Sym(LDCH), Format::F3_4_mem},
        InstructionMnemonic {Sym(LDB), Format::F3_4_mem},
        InstructionMnemonic {Sym(LDF), Format::F3_4_mem},
        InstructionMnemonic {Sym(LDL), Format::F3_4_mem},
        InstructionMnemonic {Sym(LDS), Format::F3_4_mem},
        InstructionMnemonic {Sym(LDT), Format::F3_4_mem},
        InstructionMnemonic {Sym(LDX), Format::F3_4_mem},
        InstructionMnemonic {Sym(LPS), Format::F3_4_mem},
        InstructionMnemonic {Sym(STA), Format::F3_4_mem},
        InstructionMnemonic {Sym(STCH), Format::F3_4_mem},
        InstructionMnemonic {Sym(STB), Format::F3_4_mem},
        InstructionMnemonic {Sym(STF), Format::F3_4_mem},
        InstructionMnemonic {Sym(STL), Format::F3_4_mem},
        InstructionMnemonic {Sym(STS), Format::F3_4_mem},
        InstructionMnemonic {Sym(STT), Format::F3_4_mem},
        InstructionMnemonic {Sym(STX), Format::F3_4_mem},
        InstructionMnemonic {Sym(STSW), Format::F3_4_mem},
        // Arithmetic
        InstructionMnemonic {Sym(ADD), Format::F3_4_mem},
        InstructionMnemonic {Sym(SUB), Format::F3_4_mem},
        InstructionMnemonic {Sym(MUL), Format::F3_4_mem},
        InstructionMnemonic {Sym(DIV), Format::F3_4_mem},
        InstructionMnemonic {Sym(COMP), Format::F3_4_mem},
        InstructionMnemonic {Sym(AND), Format::F3_4_mem},
        InstructionMnemonic {Sym(OR), Format::F3_4_mem},
        InstructionMnemonic {Sym(TIX), Format::F3_4_mem},
        InstructionMnemonic {Sym(ADDF), Format::F3_4_mem},
        InstructionMnemonic {Sym(SUBF), Format::F3_4_mem},
        InstructionMnemonic {Sym(MULF), Format::F3_4_mem},
        InstructionMnemonic {Sym(DIVF), Format::F3_4_mem},
        InstructionMnemonic {Sym(COMPF), Format::F3_4_mem},
        // Jump
        InstructionMnemonic {Sym(J), Format::F3_4_mem},
        InstructionMnemonic {Sym(JEQ), Format::F3_4_mem},
        InstructionMnemonic {Sym(JGT), Format::F3_4_mem},
        InstructionMnemonic {Sym(JLT), Format::F3_4_mem},
        InstructionMnemonic {Sym(JSUB), Format::F3_4_mem},
        InstructionMnemonic {Sym(RSUB), Format::F3},
        // Devices
        InstructionMnemonic {Sym(RD), Format::F3_4_mem},
        InstructionMnemonic {Sym(WD), Format::F3_4_mem},
        InstructionMnemonic {Sym(TD), Format::F3_4_mem},
        // System
        InstructionMnemonic {Sym(SSK), Format::F3_4_mem},
};
#undef Sym

#define Sym(name) Directive::name, Token::Type::name, #name
constexpr std::array directive_table{
        DirectiveMnemonic { Sym(START), Format::D_expr },
        DirectiveMnemonic { Sym(END), Format::D_expr },
        DirectiveMnemonic { Sym(EQU), Format::D_expr },
        DirectiveMnemonic { Sym(ORG), Format::D_expr },
        DirectiveMnemonic { Sym(BASE), Format::D_expr },
        DirectiveMnemonic { Sym(LTORG), Format::D },
        DirectiveMnemonic { Sym(NOBASE), Format::D },
        DirectiveMnemonic { Sym(CSECT), Format::D },
        DirectiveMnemonic { Sym(USE), Format::D_sym },
        DirectiveMnemonic { Sym(EXTDEF), Format::D_sym_array },
        DirectiveMnemonic { Sym(EXTREF), Format::D_sym_array },
        DirectiveMnemonic { Sym(RESW), Format::S_reserve },
        DirectiveMnemonic { Sym(RESB), Format::S_reserve },
        DirectiveMnemonic { Sym(BYTE), Format::S_initialize },
        DirectiveMnemonic { Sym(WORD), Format::S_initialize },
};
#undef Sym

const InstructionMnemonic* get_instruction_mnemonic(Opcode opcode) {
    static map<Opcode, InstructionMnemonic const*> mnemonics;

    if (mnemonics.empty()) {
        for (auto const& mnemonic : instruction_table) {
            mnemonics.insert_or_assign(mnemonic.opcode, &mnemonic);
        }
    }

    try {
        return mnemonics.at(opcode);
    } catch (std::out_of_range&) {
        return nullptr;
    }
}

const InstructionMnemonic* get_instruction_mnemonic(Token::Type token) {
    static map<Token::Type, InstructionMnemonic const*> mnemonics;

    if (mnemonics.empty()) {
        for (auto const& mnemonic: instruction_table) {
            mnemonics.insert_or_assign(mnemonic.token_type, &mnemonic);
        }
    }

    try {
        return mnemonics.at(token);
    } catch (std::out_of_range&) {
        return nullptr;
    }
}

const DirectiveMnemonic* get_directive_mnemonic(Directive directive) {
    static map<Directive, DirectiveMnemonic const*> mnemonics;

    if (mnemonics.empty()) {
        for (auto const& mnemonic : directive_table) {
            mnemonics.insert_or_assign(mnemonic.direcitve, &mnemonic);
        }
    }

    try {
        return mnemonics.at(directive);
    } catch (std::out_of_range&) {
        return nullptr;
    }
}

const DirectiveMnemonic* get_directive_mnemonic(Token::Type token) {
    static map<Token::Type, DirectiveMnemonic const*> mnemonics;

    if (mnemonics.empty()) {
        for (auto const& mnemonic : directive_table) {
            mnemonics.insert_or_assign(mnemonic.token_type, &mnemonic);
        }
    }

    try {
        return mnemonics.at(token);
    } catch (std::out_of_range&) {
        return nullptr;
    }
}