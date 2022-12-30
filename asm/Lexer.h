//
// Created by Lenart on 12/11/2022.
//

#ifndef ASS2_LEXER_H
#define ASS2_LEXER_H


#include <memory>
#include <string_view>
#include <optional>
#include <memory>
#include <ostream>

enum class Radix {
    Binary = 2,
    Octal = 8,
    Decimal = 10,
    Hex = 16,
    Invalid = 1
};


struct Token {
    enum Type {
        // General
        NumberBin,
        NumberOct,
        NumberDec,
        NumberHex,
        NewLine,
        Whitespace,
        Comment,
        Comma,
        Immediate,
        Indirect,
        Literal,
        CharReservation,
        HexReservation,
        Label,

        // Expressions
        Minus,
        Plus,  // Or extended
        Asterisk,
        Slash,
        OpenParenthesis,
        CloseParenthesis,

        // Garbage
        Invalid,

        // Instructions
        ADD,ADDF,ADDR,AND,CLEAR,
        COMP,COMPF,COMPR,DIV,DIVF,
        DIVR,FIX,FLOAT,HIO,J,
        JEQ,JGT,JLT,JSUB,LDA,
        LDB,LDCH,LDF,LDL,LDS,
        LDT,LDX,LPS,MUL,MULF,
        MULR,NORM,OR,RD,RMO,RSUB,
        SHIFTL,SHIFTR,SIO,SSK,STA,
        STB,STCH,STF,STI,STL,
        STS,STSW,STT,STX,SUB,
        SUBF,SUBR,SVC,TD,TIO,
        TIX,TIXR,WD,

        // Directives
        START,END,EQU,ORG,BASE,LTORG,
        RESW,RESB,BYTE,NOBASE,WORD,
        USE,EXTDEF,EXTREF,CSECT,

        // Registers
        A, X, L, B, S, T, F
    } type;

    std::string content;
    size_t line {};

    static Token invalid() { return { Invalid, {}, 0 }; }
    static Token create_from_string(const std::string_view& view, size_t line);
    static const char * type_to_str(Token::Type t);
    static Type type_from_radix(Radix r);
    [[nodiscard]] Radix to_radix() const;

    friend std::ostream &operator<<(std::ostream &os, const Token &token);
};

class Lexer {
public:
    explicit Lexer(std::string_view input_data) :
        input(input_data) {}
    std::optional<Token> next();


private:
    [[nodiscard]] bool eof() const;
    [[nodiscard]] uint8_t peek(size_t steps = 0) const;
    [[nodiscard]] uint8_t get();
    [[nodiscard]] std::string substr(size_t start, size_t end);

    static bool is_ascii_alpha(uint8_t c);
    static bool is_ascii_digit(uint8_t c);
    static bool is_ascii_hexdigit(uint8_t c);
    static bool is_ascii_octdigit(uint8_t c);
    static bool is_ascii_binary(uint8_t c);
    static bool is_ascii_alphanumeric(uint8_t c);
    static bool is_whitespace(uint8_t c);
    static bool is_digit_of_radix(uint8_t c, Radix radix);

private:
    Token lex_simple(Token::Type type);
    Token lex_whitespace();
    Token lex_number();
    Token lex_comment();
    Token lex_word_or_number();
    Token lex_reservation();

private:
    size_t index = 0;
    size_t line = 1;
    std::string_view input;
};


#endif //ASS2_LEXER_H
