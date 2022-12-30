//
// Created by Lenart on 05/12/2022.
//

#ifndef ASS2_DISASSEMBLER_H
#define ASS2_DISASSEMBLER_H


#include "Memory.h"

class Disassembler {



private:
public:
    explicit Disassembler(std::shared_ptr<Memory> memory);

    std::string dissasemble_at(Address_t address);

private:
    std::shared_ptr<Memory> m_memory;
};


#endif //ASS2_DISASSEMBLER_H
