//
// Created by Lenart on 04/12/2022.
//

#ifndef ASS2_FLAGS_H
#define ASS2_FLAGS_H


#include <cstdint>
#include "SicTypes.h"

class Flags {

public:
    Flags() = default;
    Flags(uint8_t ni, uint8_t xbpe);

    [[nodiscard]] bool is_sic() const;
    [[nodiscard]] bool is_immediate() const;
    [[nodiscard]] bool is_indirect() const;
    [[nodiscard]] bool is_direct() const;
    [[nodiscard]] bool is_simple() const;
    [[nodiscard]] bool is_indexed() const;
    [[nodiscard]] bool is_pc_relative() const;
    [[nodiscard]] bool is_base_relative() const;
    [[nodiscard]] bool is_absolute() const;
    [[nodiscard]] bool is_extended() const;
    [[nodiscard]] bool is_valid() const;
    [[nodiscard]] Byte_t get_ni() const;
    [[nodiscard]] Byte_t get_xbpe() const;

    void set_indirect(bool v);
    void set_immediate(bool v);
    void set_indexed(bool v);
    void set_base_relative(bool v);
    void set_pc_relative(bool v);
    void set_extended(bool v);

    void set_ni(uint8_t ni);
    void set_xbpe(uint8_t xbpe);
private:
    bool n: 1 {false};
    bool i: 1 {false};
    bool x: 1 {false};
    bool b: 1 {false};
    bool p: 1 {false};
    bool e: 1 {false};
};


#endif //ASS2_FLAGS_H
