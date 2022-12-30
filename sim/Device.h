//
// Created by Lenart on 05/12/2022.
//

#ifndef ASS2_DEVICE_H
#define ASS2_DEVICE_H

#include <fstream>


#include "../common/SicTypes.h"

class Device {
public:
    virtual ~Device() = default;
    virtual bool test() = 0;
    virtual Byte_t read() = 0;
    virtual void write(Byte_t b) = 0;
};

class StdinDevice : public Device {
public:
    bool test() override;
    Byte_t read() override;
    void write(Byte_t b) override;
};

class StdoutDevice : public Device {
public:
    bool test() override;
    Byte_t read() override;
    void write(Byte_t b) override;
};

class StderrDevice : public Device {
public:
    bool test() override;
    Byte_t read() override;
    void write(Byte_t b) override;
};

class FileDevice : public Device {
public:
    explicit FileDevice(Byte_t id, bool clear_file);

    bool test() override;
    Byte_t read() override;
    void write(Byte_t b) override;

private:
    std::fstream m_file_stream {};
};


#endif //ASS2_DEVICE_H
