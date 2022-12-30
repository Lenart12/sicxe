//
// Created by Lenart on 04/12/2022.
//

#ifndef ASS2_REGISTERS_H
#define ASS2_REGISTERS_H

#include <ostream>
#include "../common/SicTypes.h"

struct RegisterChange {
    Register register_id;
    Register_t previous_value;
    Register_t new_value;
    
    friend std::ostream &operator<<(std::ostream &os, const RegisterChange &change);
};
class Registers {
public:
    Registers() = default;
    [[nodiscard]] Register_t get(Register reg) const;
    RegisterChange set(Register reg, Register_t new_value);

    [[nodiscard]] Register_t getA() const;
    RegisterChange setA(Register_t a);
    [[nodiscard]] Register_t getX() const;
    RegisterChange setX(Register_t x);
    [[nodiscard]] Register_t getL() const;
    RegisterChange setL(Register_t l);
    [[nodiscard]] Register_t getB() const;
    RegisterChange setB(Register_t b);
    [[nodiscard]] Register_t getS() const;
    RegisterChange setS(Register_t s);
    [[nodiscard]] Register_t getT() const;
    RegisterChange setT(Register_t t);
    [[nodiscard]] Register_t getF() const;
    RegisterChange setF(Register_t f);
    [[nodiscard]] Address_t getPc() const;
    RegisterChange setPc(Address_t pc);
    [[nodiscard]] Register_t getSw() const;
    RegisterChange setSw(Register_t sw);
    [[nodiscard]] Register_t getCC() const;
    RegisterChange setCC(Register_t cc);
    [[nodiscard]] bool CC_is_greater() const;
    [[nodiscard]] bool CC_is_lower() const;
    [[nodiscard]] bool CC_is_equal() const;

    void undo(RegisterChange change);


private:
    Register_t m_A  {};
    Register_t m_X  {};
    Register_t m_L  {};
    Register_t m_B  {};
    Register_t m_S  {};
    Register_t m_T  {};
    Register_t m_F  {};
    Address_t m_PC {};
    Register_t m_SW {};
};




#endif //ASS2_REGISTERS_H
