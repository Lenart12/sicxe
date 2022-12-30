//
// Created by Lenart on 20/11/2022.
//

#ifndef ASS2_MNEMONICS_H
#define ASS2_MNEMONICS_H

#include <map>
#include <cassert>
#include "../asm/Lexer.h"

enum class Opcode {
    LDA  = 0x00,    LDX   = 0x04,    LDL   = 0x08,    STA    = 0x0C,    STX    = 0x10,
    STL  = 0x14,    ADD   = 0x18,    SUB   = 0x1C,    MUL    = 0x20,    DIV    = 0x24,
    COMP = 0x28,    TIX   = 0x2C,    JEQ   = 0x30,    JGT    = 0x34,    JLT    = 0x38,
    J    = 0x3C,    AND   = 0x40,    OR    = 0x44,    JSUB   = 0x48,    RSUB   = 0x4C,
    LDCH = 0x50,    STCH  = 0x54,    ADDF  = 0x58,    SUBF   = 0x5C,    MULF   = 0x60,
    DIVF = 0x64,    COMPF = 0x88,    LDB   = 0x68,    LDS    = 0x6C,    LDF    = 0x70,
    LDT  = 0x74,    STB   = 0x78,    STS   = 0x7C,    STF    = 0x80,    STT    = 0x84,
    LPS  = 0xD0,    STI   = 0xD4,    STSW  = 0xE8,    RD     = 0xD8,    WD     = 0xDC,
    TD   = 0xE0,    SSK   = 0xEC,    FLOAT = 0xC0,    FIX    = 0xC4,    NORM   = 0xC8,
    SIO  = 0xF0,    HIO   = 0xF4,    TIO   = 0xF8,    ADDR   = 0x90,    SUBR   = 0x94,
    MULR = 0x98,    DIVR  = 0x9C,    COMPR = 0xA0,    SHIFTL = 0xA4,    SHIFTR = 0xA8,
    RMO  = 0xAC,    SVC   = 0xB0,    CLEAR = 0xB4,    TIXR   = 0xB8,
};

enum class Directive {
    START,END,EQU,ORG,BASE,LTORG,
    RESW,RESB,BYTE,NOBASE,WORD,
    USE,EXTDEF,EXTREF,CSECT
};

enum class Format {
    // Format 1 (1 byte)
    F1,
    // Format 2 (2 bytes)
    F2_num,
    F2_reg,
    F2_reg_num,
    F2_reg_reg,
    // Format 3/4 (3/4 bytes)
    F3,
    F3_4_mem,
    // Directives
    D,
    D_expr,
    D_sym,
    D_sym_array,
    // Storage
    S_reserve,
    S_initialize

};

struct InstructionMnemonic {
    Opcode opcode;
    Token::Type token_type;
    std::string_view mnemonic;
    Format format;
};

struct DirectiveMnemonic {
    Directive direcitve;
    Token::Type token_type;
    std::string_view mnemonic;
    Format format;
};

[[nodiscard]] const InstructionMnemonic* get_instruction_mnemonic(Opcode opcode);
[[nodiscard]] const InstructionMnemonic* get_instruction_mnemonic(Token::Type token);
[[nodiscard]] const DirectiveMnemonic* get_directive_mnemonic(Directive directive);
[[nodiscard]] const DirectiveMnemonic* get_directive_mnemonic(Token::Type token);


#endif //ASS2_MNEMONICS_H
