//
// Created by Lenart on 04/12/2022.
//

#include "Flags.h"

Flags::Flags(uint8_t ni, uint8_t xbpe) {
    set_ni(ni & 0b11);
    set_xbpe(xbpe & 0b1111);
}

bool Flags::is_sic() const {
    return !i && !n;
}

bool Flags::is_immediate() const {
    return i && !n;
}

bool Flags::is_indirect() const {
    return n && !i;
}

bool Flags::is_direct() const {
    return i && n;
}

bool Flags::is_simple() const {
    return is_sic() || is_direct();
}

bool Flags::is_indexed() const {
    return x;
}

bool Flags::is_pc_relative() const {
    if (is_sic()) return false;
    return p;
}

bool Flags::is_base_relative() const {
    if (is_sic()) return false;
    return b;
}

bool Flags::is_absolute() const {
    if (is_sic()) return true;
    return !(is_base_relative() || is_pc_relative());
}

bool Flags::is_extended() const {
    if (is_sic()) return false;
    return e;
}

bool Flags::is_valid() const {
    if (is_sic()) return true;

    if (is_indexed() && !is_simple()) return false;
    if (is_pc_relative() && is_base_relative()) return false;

    return true;
}

void Flags::set_xbpe(uint8_t xbpe) {
    x = xbpe & 0b1000;
    b = xbpe & 0b0100;
    p = xbpe & 0b0010;
    e = xbpe & 0b0001;
}

void Flags::set_ni(uint8_t ni) {
    n = ni & 0b10;
    i = ni & 0b01;
}

void Flags::set_indirect(bool v) {
    n = v;
}

void Flags::set_immediate(bool v) {
    i = v;
}

void Flags::set_indexed(bool v) {
    x = v;
}

void Flags::set_base_relative(bool v) {
    b = v;
}

void Flags::set_pc_relative(bool v) {
    p = v;
}

void Flags::set_extended(bool v) {
    e = v;
}

Byte_t Flags::get_ni() const {
    return (n << 1) | i;
}

Byte_t Flags::get_xbpe() const {
    return (x << 3) | (b << 2) | (p << 1) | e;
}


