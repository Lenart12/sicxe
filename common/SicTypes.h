//
// Created by Lenart on 04/12/2022.
//

#ifndef ASS2_SICTYPES_H
#define ASS2_SICTYPES_H


#include <cstdint>
#include <string_view>
#include <cassert>
#include <variant>
#include <memory>
#include <vector>
#include <sstream>
#include "../asm/ast/Forward.h"

using std::string;
using std::stringstream;
using std::variant;
using std::shared_ptr;
using std::vector;

using Register_t = int;
using Address_t = uint32_t;
using Byte_t = uint8_t;
using Word_t = uint32_t;
using Float_t = double;

enum class Register {
    A, X, L, B, S,
    T, F, PC, SW, CC
};

struct OperandsNone {};
struct OperandsReg {
    Register reg;
};
struct OperandsNum {
    int num;
};
struct OperandsRegNum {
    Register reg;
    int num;
};
struct OperandsRegReg {
    Register reg_1;
    Register reg_2;
};
struct OperandsExpr {
    shared_ptr<Ast::Expression> expression;
};
struct OperandsSymbol {
    string symbol;
};

struct OperandsSymbolArray {
    vector<string> symbols;
};

struct OperandsByteArray {
    vector<Byte_t> bytes;
};

//using Operand_t = variant<OperandsNone, OperandsReg, OperandsNum, OperandsRegNum, OperandsRegReg, OperandsExpr>;
using Operand_t = variant<
        OperandsNone,
        OperandsReg,
        OperandsNum,
        OperandsRegNum,
        OperandsRegReg,
        OperandsExpr,
        OperandsSymbol,
        OperandsSymbolArray,
        OperandsByteArray
    >;

std::string_view register_to_str(Register r);

string operand_to_str(Operand_t const& op);



#endif //ASS2_SICTYPES_H
