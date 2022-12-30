//
// Created by Lenart on 28/12/2022.
//

#include <iomanip>
#include "SicTypes.h"
#include "../asm/ast/SicAST.h"

std::string_view register_to_str(Register r) {
#define r_str(reg) case Register::reg: return #reg;
    switch (r) {
        r_str(A) r_str(X) r_str(L) r_str(B) r_str(S)
        r_str(T) r_str(F) r_str(PC) r_str(SW) r_str(CC)
    }
#undef r_str

    assert(!"Invalid register");
    return {};
}

template<class>
inline constexpr bool always_false_v = false;
string operand_to_str(Operand_t const& op) {
    stringstream ss;
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, OperandsNone>) {}
        else if constexpr (std::is_same_v<T, OperandsReg>) {
            ss << register_to_str(arg.reg);
        }
        else if constexpr (std::is_same_v<T, OperandsNum>) {
            ss << arg.num;
        }
        else if constexpr (std::is_same_v<T, OperandsRegNum>) {
            ss << register_to_str(arg.reg) << ", " << arg.num;
        }
        else if constexpr (std::is_same_v<T, OperandsRegReg>) {
            ss << register_to_str(arg.reg_1) << ", " << register_to_str(arg.reg_1);
        }
        else if constexpr (std::is_same_v<T, OperandsExpr>) {
            ss << arg.expression->to_string();
        }
        else if constexpr (std::is_same_v<T, OperandsSymbol>) {
            ss << arg.symbol;
        }
        else if constexpr (std::is_same_v<T, OperandsSymbolArray>) {
            auto const& symbols = arg.symbols;

            for (int i = 0; i < symbols.size(); i++) {
                ss << symbols[i];
                if (i < symbols.size() - 1) {
                    ss << ", ";
                }
            }
        }
        else if constexpr (std::is_same_v<T, OperandsByteArray>) {
            ss << "X'";
            for (auto const& byte : arg.bytes) {
                ss << std::setw(2) << std::setfill('0') << std::hex << (int)byte;
            }
            ss << '\'';
        }
        else
            static_assert(always_false_v<T>, "non-exhaustive visitor!");
    }, op);

    return ss.str();
}