//
// Created by Lenart on 12/11/2022.
//

#include "ObjLoader.h"
#include <string>
#include <fstream>
#include <cassert>
#include <iostream>

Address_t ObjLoader::load_obj() {
    skip_whitespace();

    if (m_input.get() != 'H') {
        assert(!"Invalid format");
    }

    auto name = read_string(6);
    auto section_start = read_word();
    auto section_length = read_word();
    auto section_end = section_start + section_length;

    skip_whitespace();

    while (m_input.get() == 'T') {
        auto addr = read_word();
        auto length = read_byte();

        while (length--) {
            if (addr < section_start || addr >= section_end) return {};
            m_memory->set_byte(addr++, read_byte());
        }

        skip_whitespace();
    }

    m_input.unget();

    if (m_input.peek() == 'M') {
        while (m_input.get() == 'M') {
            auto addr = read_word();
            auto len = read_byte();

            assert(len == 5 && "Other modifications not supported");

            auto to_fix = m_memory->get_word(addr);
            auto upper_nibble = to_fix & 0xf00000;
            to_fix &= 0xfffff;
            // FIXME:
            // to_fix += section_start;
            to_fix |= upper_nibble;
            m_memory->set_word(addr, to_fix);

            skip_whitespace();
        }
        m_input.unget();
    }

    if (m_input.get() != 'E') {
        assert(!"Invalid format");
    }

    return read_word();
}

std::string ObjLoader::read_string(size_t n) {
    skip_whitespace();
    std::string out {};
    while (n--) {
        char c;
        m_input.read(&c, 1);
        out.push_back(c);
    }
    return out;
}

uint32_t ObjLoader::read_word() {
    auto word = read_string(6);
    return std::strtol(word.c_str(), nullptr, 16);
}

uint8_t ObjLoader::read_byte() {
    auto word = read_string(2);
    return std::strtol(word.c_str(), nullptr, 16);
}

void ObjLoader::skip_whitespace() {
    while (m_input.peek() == ' ' || m_input.peek() == '\n' || m_input.peek() == '\r' || m_input.peek() == '\t') m_input.get();
}
