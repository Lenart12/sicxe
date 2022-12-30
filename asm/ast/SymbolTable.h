//
// Created by Lenart on 28/12/2022.
//

#ifndef ASS2_SYMBOLTABLE_H
#define ASS2_SYMBOLTABLE_H

#include <vector>
#include <string>
#include <optional>
#include "../../common/SicTypes.h"

using std::vector;
using std::optional;

class SymbolTable {
public:
    struct Symbol {
        string label;
        Address_t address;
    };

    void define_symbol(Symbol s);
    optional<Symbol> find_symbol(string const& name);

    vector<Symbol>& get_table();
private:
    vector<Symbol> symbols;
};


#endif //ASS2_SYMBOLTABLE_H
