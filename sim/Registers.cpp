//
// Created by Lenart on 04/12/2022.
//

#include <cassert>
#include <iomanip>
#include "Registers.h"

using enum Register;

Register_t Registers::get(Register reg) const {
    switch (reg) {
        case A:  return m_A;
        case X:  return m_X;
        case L:  return m_L;
        case B:  return m_B;
        case S:  return m_S;
        case T:  return m_T;
        case F:  return m_F;
        case PC: return m_PC;
        case SW: return m_SW;
        case CC: return getCC();
        default: assert(!"Invalid register id to get");
    }
    return 0;
}

RegisterChange Registers::set(Register reg, Register_t new_value) {
    if (new_value & 0x800000) new_value |= 0xff000000; // NOLINT(cppcoreguidelines-narrowing-conversions)
    else new_value &= ~0xff000000; // NOLINT(cppcoreguidelines-narrowing-conversions)

    RegisterChange change {
        .register_id = reg,
        .previous_value = get(reg),
        .new_value = new_value
    };

    switch (reg) {
        case A:  m_A = new_value;  break;
        case X:  m_X = new_value;  break;
        case L:  m_L = new_value;  break;
        case B:  m_B = new_value;  break;
        case S:  m_S = new_value;  break;
        case T:  m_T = new_value;  break;
        case F:  m_F = new_value;  break;
        case PC: m_PC = new_value; break;
        case SW: m_SW = new_value; break;
        case CC: return setCC(new_value);
        default: assert(!"Invalid register id to set");
    }
    return change;
}

Register_t Registers::getA() const {
    return get(A);
}

RegisterChange Registers::setA(Register_t a) {
    return set(A, a);
}

Register_t Registers::getX() const {
    return get(X);
}

RegisterChange Registers::setX(Register_t x) {
    return set(X, x);
}

Register_t Registers::getL() const {
    return get(L);
}

RegisterChange Registers::setL(Register_t l) {
    return set(L, l);
}

Register_t Registers::getB() const {
    return get(B);
}

RegisterChange Registers::setB(Register_t b) {
    return set(B, b);
}

Register_t Registers::getS() const {
    return get(S);
}

RegisterChange Registers::setS(Register_t s) {
    return set(S, s);
}

Register_t Registers::getT() const {
    return get(T);
}

RegisterChange Registers::setT(Register_t t) {
    return set(T, t);
}

Register_t Registers::getF() const {
    return get(F);
}

RegisterChange Registers::setF(Register_t f) {
    return set(F, f);
}

Address_t Registers::getPc() const {
    return get(PC);
}

RegisterChange Registers::setPc(Address_t pc) {
    return set(PC, static_cast<Register_t>(pc));
}

Register_t Registers::getSw() const {
    return get(SW);
}

RegisterChange Registers::setSw(Register_t sw) {
    return set(SW, sw);
}

Register_t Registers::getCC() const {
    return (get(SW) & 0b1100) >> 2;
}

RegisterChange Registers::setCC(Register_t cc) {
    if (cc < 0) cc = 0b01;
    else if (cc > 0) cc = 0b10;
    auto sw = getSw();
    sw = (sw & ~(0b1100)) | (cc << 2);
    return set(SW, sw);
}

bool Registers::CC_is_greater() const {
    return getCC() == 0b10;
}

bool Registers::CC_is_lower() const {
    return getCC() == 0b01;
}

bool Registers::CC_is_equal() const {
    return getCC() == 0b00;
}

void Registers::undo(RegisterChange change) {
    set(change.register_id, change.previous_value);
}

std::ostream &operator<<(std::ostream &os, const RegisterChange &change) {
    os << "register: " << std::setfill(' ') << std::setw(2) << register_to_str(change.register_id) << " previous_value: 0x";
    os << std::setfill('0') << std::setw(6) << std::hex << (change.previous_value & 0xffffff);
    os << " new_value: 0x";
    os << std::setfill('0') << std::setw(6) << std::hex << (change.new_value & 0xffffff);
    return os;
}
