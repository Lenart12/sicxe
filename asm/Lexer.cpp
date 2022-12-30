//
// Created by Lenart on 12/11/2022.
//

#include <iostream>
#include "Lexer.h"

Token Token::create_from_string(const std::string_view& view, size_t line) {
    #define  str_to_tok(str) \
        else if (view == #str) return {.type = str, .content = std::string {view}, .line = line };

    // Start compare chain for str_to_tok
    if (false) {}

    // Instructions
    str_to_tok(ADD) str_to_tok(ADDF) str_to_tok(ADDR) str_to_tok(AND) str_to_tok(CLEAR) str_to_tok(COMP)
    str_to_tok(COMPF) str_to_tok(COMPR) str_to_tok(DIV) str_to_tok(DIVF) str_to_tok(DIVR)
    str_to_tok(FIX) str_to_tok(FLOAT) str_to_tok(HIO) str_to_tok(J) str_to_tok(JEQ)
    str_to_tok(JGT) str_to_tok(JLT) str_to_tok(JSUB) str_to_tok(LDA) str_to_tok(LDB)
    str_to_tok(LDCH) str_to_tok(LDF) str_to_tok(LDL) str_to_tok(LDS) str_to_tok(LDT)
    str_to_tok(LDX) str_to_tok(LPS) str_to_tok(MUL) str_to_tok(MULF) str_to_tok(MULR)
    str_to_tok(NORM) str_to_tok(OR) str_to_tok(RD) str_to_tok(RMO) str_to_tok(RSUB)
    str_to_tok(SHIFTL) str_to_tok(SHIFTR) str_to_tok(SIO) str_to_tok(SSK) str_to_tok(STA)
    str_to_tok(STB) str_to_tok(STCH) str_to_tok(STF) str_to_tok(STI) str_to_tok(STL)
    str_to_tok(STS) str_to_tok(STSW) str_to_tok(STT) str_to_tok(STX) str_to_tok(SUB)
    str_to_tok(SUBF) str_to_tok(SUBR) str_to_tok(SVC) str_to_tok(TD) str_to_tok(TIO)
    str_to_tok(TIX) str_to_tok(TIXR) str_to_tok(WD)

    // Directives
    str_to_tok(START) str_to_tok(END) str_to_tok(EQU) str_to_tok(ORG) str_to_tok(BASE)
    str_to_tok(LTORG) str_to_tok(RESW) str_to_tok(RESB) str_to_tok(BYTE) str_to_tok(NOBASE) str_to_tok(WORD)
    str_to_tok(USE) str_to_tok(EXTDEF) str_to_tok(EXTREF) str_to_tok(CSECT)

    // Registers
    str_to_tok(A) str_to_tok(X) str_to_tok(L) str_to_tok(B) str_to_tok(S) str_to_tok(T) str_to_tok(F)

    #undef str_to_tok

    else return { .type = Label, .content = std::string { view }, .line = line };
}

const char * Token::type_to_str(Token::Type t) {
    #define type_to_str(type) case type: return #type;
    switch (t) {
        // General
        type_to_str(NumberBin) type_to_str(NumberOct) type_to_str(NumberDec) type_to_str(NumberHex) type_to_str(NewLine)
        type_to_str(Whitespace) type_to_str(Comment) type_to_str(Comma) type_to_str(Immediate) type_to_str(Indirect)
        type_to_str(Literal) type_to_str(CharReservation) type_to_str(HexReservation) type_to_str(Label)
        type_to_str(Invalid)

        // Expresions
        type_to_str(Minus) type_to_str(Plus) type_to_str(Asterisk) type_to_str(Slash)
        type_to_str(OpenParenthesis) type_to_str(CloseParenthesis)

        // InstructionMnemonic
        type_to_str(ADD) type_to_str(ADDF) type_to_str(ADDR) type_to_str(AND)
        type_to_str(CLEAR) type_to_str(COMP) type_to_str(COMPF) type_to_str(COMPR) type_to_str(DIV)
        type_to_str(DIVF) type_to_str(DIVR) type_to_str(FIX) type_to_str(FLOAT) type_to_str(HIO)
        type_to_str(J) type_to_str(JEQ) type_to_str(JGT) type_to_str(JLT) type_to_str(JSUB)
        type_to_str(LDA) type_to_str(LDB) type_to_str(LDCH) type_to_str(LDF) type_to_str(LDL)
        type_to_str(LDS) type_to_str(LDT) type_to_str(LDX) type_to_str(LPS) type_to_str(MUL)
        type_to_str(MULF) type_to_str(MULR) type_to_str(NORM) type_to_str(OR) type_to_str(RD)
        type_to_str(RMO) type_to_str(RSUB) type_to_str(SHIFTL) type_to_str(SHIFTR) type_to_str(SIO)
        type_to_str(SSK) type_to_str(STA) type_to_str(STB) type_to_str(STCH) type_to_str(STF)
        type_to_str(STI) type_to_str(STL) type_to_str(STS) type_to_str(STSW) type_to_str(STT)
        type_to_str(STX) type_to_str(SUB) type_to_str(SUBF) type_to_str(SUBR) type_to_str(SVC)
        type_to_str(TD) type_to_str(TIO) type_to_str(TIX) type_to_str(TIXR) type_to_str(WD)

        // Directives
        type_to_str(START) type_to_str(END) type_to_str(EQU) type_to_str(ORG) type_to_str(BASE)
        type_to_str(LTORG) type_to_str(RESW) type_to_str(RESB) type_to_str(BYTE) type_to_str(NOBASE) type_to_str(WORD)
        type_to_str(USE) type_to_str(EXTDEF) type_to_str(EXTREF) type_to_str(CSECT)

        // Registers
        type_to_str(A) type_to_str(X) type_to_str(L) type_to_str(B) type_to_str(S)
        type_to_str(T) type_to_str(F)
    }
    #undef type_to_str
    return "";
}

Token::Type Token::type_from_radix(Radix r) {
    switch (r) {
        case Radix::Binary:
            return NumberBin;
        case Radix::Octal:
            return NumberOct;
        case Radix::Decimal:
            return NumberDec;
        case Radix::Hex:
            return NumberHex;
    }
    return Invalid;
}

Radix Token::to_radix() const {
    switch (type) {
        case NumberBin:
            return Radix::Binary;
        case NumberOct:
            return Radix::Octal;
        case NumberDec:
            return Radix::Decimal;
        case NumberHex:
            return Radix::Hex;
    }
    return Radix::Invalid;
}

std::ostream &operator<<(std::ostream &os, const Token &token) {
    os << "type: " << Token::type_to_str(token.type) << " content: " << token.content;
    return os;
}



bool Lexer::eof() const {
    return index >= input.size();
}

uint8_t Lexer::peek(size_t steps) const {
    steps += index;
    if (steps >= input.size()) return 0;
    return input[steps];
}

uint8_t Lexer::get() {
    auto c = peek(0);
    index += 1;
    return c;
}



bool Lexer::is_ascii_alpha(uint8_t c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::is_ascii_digit(uint8_t c) {
    return c >= '0' && c <= '9';
}

bool Lexer::is_ascii_hexdigit(uint8_t c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool Lexer::is_ascii_octdigit(uint8_t c) {
    return c >= '0' && c <= '7';
}

bool Lexer::is_ascii_binary(uint8_t c) {
    return c == '0' || c == '1';
}

bool Lexer::is_ascii_alphanumeric(uint8_t c) {
    return is_ascii_alpha(c) || is_ascii_digit(c);
}

bool Lexer::is_whitespace(uint8_t c) {
    return c == ' ' || c == '\t' || c == '\r';
}

bool Lexer::is_digit_of_radix(uint8_t c, Radix radix) {
    switch (radix) {
        case Radix::Binary:
            return is_ascii_binary(c);
        case Radix::Octal:
            return is_ascii_octdigit(c);
        case Radix::Decimal:
            return is_ascii_digit(c);
        case Radix::Hex:
            return is_ascii_hexdigit(c);
    }
    return false;
}

std::string Lexer::substr(size_t start, size_t end) {
    return std::string { input.substr(start, end - start) };
}


std::optional<Token> Lexer::next() {
    if (eof()) return {};
    // Skip whitespace or to the end
    if (is_whitespace(peek())) return lex_whitespace();

    auto start = index;
    switch (peek()) {
        case '#': return lex_simple(Token::Immediate);
        case '@': return lex_simple(Token::Indirect);
        case ',': return lex_simple(Token::Comma);
        case '=': return lex_simple(Token::Literal);
        case '-': return lex_simple(Token::Minus);
        case '+': return lex_simple(Token::Plus);
        case '*': return lex_simple(Token::Asterisk);
        case '/': return lex_simple(Token::Slash);
        case '(': return lex_simple(Token::OpenParenthesis);
        case ')': return lex_simple(Token::CloseParenthesis);
        case '\n': line++; return lex_simple(Token::NewLine);
        case '.': return lex_comment();
        case 'C':
        case 'X': {
            if (peek(1) == '\'') return lex_reservation();
            return lex_word_or_number();
        }
        default: return lex_word_or_number();
    }
}

Token Lexer::lex_number() {
    auto start = index;
    auto radix = Radix::Decimal;

    if (peek() == '0') {
        switch (peek(1)) {
            case 'b':
                radix = Radix::Binary;
                index += 2;
                break;
            case 'o':
                radix = Radix::Octal;
                index += 2;
                break;
            case 'x':
                radix = Radix::Hex;
                index += 2;
                break;
        }
    }

    while (!eof()) {
        auto value = peek();
        if (!is_digit_of_radix(value, radix)) break;

        index++;
    }

    return Token {.type=Token::type_from_radix(radix), .content=substr(start, index), .line = line};
}

Token Lexer::lex_comment() {
    auto start = index;
    while (!eof() && peek() != '\n') index++;
    return {.type=Token::Comment, .content=substr(start, index), .line = line};
}

Token Lexer::lex_word_or_number() {

    if (is_ascii_digit(peek())) return lex_number();

    auto start = index;
    while (!eof() && is_ascii_alphanumeric(peek())) index++;

    if (start == index) {
        std::cout << "Unhandled lex at index " << index << ": " << peek() << std::endl;
        exit(1);
    }
    return Token::create_from_string(substr(start, index), line);
}

Token Lexer::lex_whitespace() {
    auto start = index;
    while (!eof() && is_whitespace(peek())) index++;

    return { .type=Token::Whitespace, .content=substr(start, index), .line = line};
}

Token Lexer::lex_reservation() {
    auto reservation_type = Token::Invalid;
    auto start = index;

    switch (get()) {
        case 'C':
            reservation_type = Token::CharReservation;
            break;
        case 'X':
            reservation_type = Token::HexReservation;
            break;
    }
    if (get() != '\'') {
        std::cout << "Lex expected first \' but got " << index-1 << ": " << peek(-1) << std::endl;
        exit(1);
    }

    while (!eof() && peek() != '\'') {
        if (reservation_type == Token::CharReservation) {
            if (is_ascii_alphanumeric(peek())) index++;
            else break;
        } else {
            if (is_ascii_hexdigit(peek())) index++;
            else break;
        }
    }

    if (get() != '\'') {
        std::cout << "Lex expected second \' but got " << index-1 << ": " << peek(-1) << std::endl;
    }

    return { .type = reservation_type, .content=substr(start, index), .line = line };
}

Token Lexer::lex_simple(Token::Type type) {
    index++;
    return Token { .type = type, .content=substr(index - 1, index), .line = line };
}



