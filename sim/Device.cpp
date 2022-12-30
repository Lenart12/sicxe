//
// Created by Lenart on 05/12/2022.
//

#include <iostream>
#include <iomanip>
#include "Device.h"

bool StdoutDevice::test() {
    return true;
}

Byte_t StdoutDevice::read() {
    return 0;
}

void StdoutDevice::write(Byte_t b) {
    std::cout << b << std::flush;
}

bool StderrDevice::test() {
    return true;
}

Byte_t StderrDevice::read() {
    return 0;
}

void StderrDevice::write(Byte_t b) {
    std::cerr << b << std::flush;
}

bool StdinDevice::test() {
    return true;
}

Byte_t StdinDevice::read() {
    if (!std::cin.rdbuf()->in_avail()) {
        std::cout << ">" << std::flush;
    }
    return getc(stdin);
}

void StdinDevice::write(Byte_t b) {

}

FileDevice::FileDevice(Byte_t id, bool clear_file) {
    std::stringstream filename {};
    filename << "./" << std::setw(2) << std::setfill('0') << std::hex << (int)id  << ".dev";
    auto open_mode = std::ios::binary | std::ios::out | std::ios::in;
    if (clear_file) open_mode |= std::ios::trunc;
    m_file_stream = std::fstream {filename.str(), open_mode};
}


bool FileDevice::test() {
    return m_file_stream.good();
}

Byte_t FileDevice::read() {
    return m_file_stream.get();
}

void FileDevice::write(Byte_t b) {
    m_file_stream.put(*reinterpret_cast<char*>(&b));
    m_file_stream.flush();
}

