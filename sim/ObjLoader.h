//
// Created by Lenart on 12/11/2022.
//

#ifndef ASS2_OBJLOADER_H
#define ASS2_OBJLOADER_H

#include <istream>
#include <optional>
#include <memory>
#include "Memory.h"

class ObjLoader {
public:
    explicit ObjLoader(std::shared_ptr<Memory> memory, std::ifstream &mInput)
    : m_memory(std::move(memory)), m_input(mInput) {}

    Address_t load_obj();

private:
    void skip_whitespace();
    std::string read_string(size_t n);
    uint32_t read_word();
    uint8_t read_byte();

private:
    std::shared_ptr<Memory> m_memory;
    std::ifstream& m_input;
};


#endif //ASS2_OBJLOADER_H
