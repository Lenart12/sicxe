//
// Created by Lenart on 28/12/2022.
//

#include "SymbolTable.h"


optional<SymbolTable::Symbol> SymbolTable::find_symbol(string const& name) {
    for (auto const& symbol : symbols) {
        if (symbol.label == name) return symbol;
    }
    return std::nullopt;
}

void SymbolTable::define_symbol(SymbolTable::Symbol s) {
    symbols.push_back(std::move(s));
}

vector<SymbolTable::Symbol>& SymbolTable::get_table() {
    return symbols;
}