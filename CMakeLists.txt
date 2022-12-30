cmake_minimum_required(VERSION 3.23)
project(ass2)

set(CMAKE_CXX_STANDARD 20)

add_executable(
        ass2
        main.cpp
        asm/Lexer.cpp
        asm/Lexer.h
        asm/Parser.cpp
        asm/Parser.h
        asm/ast/SicAST.cpp
        asm/ast/SicAST.h
        sim/ObjLoader.cpp
        sim/ObjLoader.h
        sim/Memory.cpp
        sim/Memory.h
        sim/Machine.cpp
        sim/Machine.h
        common/Mnemonics.cpp
        common/Mnemonics.h
        sim/Registers.cpp
        sim/Registers.h
        common/SicTypes.h
        common/Flags.cpp
        common/Flags.h
        sim/Device.cpp
        sim/Device.h
        sim/Disassembler.cpp
        sim/Disassembler.h
        asm/ast/Visitor.cpp
        asm/ast/Visitor.h
        asm/ast/Forward.h
        asm/SicCST.cpp
        asm/SicCST.h
        asm/ast/SymbolTable.cpp
        asm/ast/SymbolTable.h
        common/SicTypes.cpp)
