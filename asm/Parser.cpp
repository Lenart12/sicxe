//
// Created by Lenart on 12/11/2022.
//

#include <iostream>
#include "Parser.h"

using std::make_shared;
using std::stringstream;

#define PARSER_TRACE() \
    if (debug_print) std::cout <<  __PRETTY_FUNCTION__ << std::endl

shared_ptr<Ast::Program> Parser::parse() {
    if (debug_print) {
        for (auto const& token : m_input) {
            std::cout << "Token::" << Token::type_to_str(token.type) << "[" << token.content << "]" << std::endl;
        }
    }

    return parse_program();
}

bool Parser::eof(size_t steps) const {
    return index + steps >= m_input.size();
}

Token Parser::peek(size_t steps) const {
    return at(index + steps);
}

Token Parser::get() {
    auto t = peek(0);
    index += 1;
    return t;
}

Token::Type Parser::peek_type(size_t steps) const {
    return peek(steps).type;
}

Token::Type Parser::get_type() {
    return get().type;
}

Token Parser::at(size_t _index) const {
    if (_index >= m_input.size()) {
        if (debug_print) {
            std::cerr << "Invalid token access: " << _index << std::endl;
            __asm__ volatile("int $0x03");
        }
        return Token::invalid();
    }
    if (debug_print) {
        std::cout << "Token [" << _index << "]: " << Token::type_to_str(m_input[_index].type);
        std::cout << "(line " << m_input[_index].line << ")" << std::endl;
    }
    return m_input[_index];
}

Token::Type Parser::at_type(size_t _index) const {
    return at(_index).type;
}

shared_ptr<Ast::Program> Parser::parse_program() {
    PARSER_TRACE();
    auto program = make_shared<Ast::Program>();

    while (!eof()) {
        while (is_empty_line()) skip_to_next_line();


        optional<string> section_name;
        auto start = index;
        if (match_start_of_section()) {
            section_name = parse_start_of_section();
        }

        if (program->contains_section(section_name)) {
            unexpected(at(start), "Duplicate section names");
        }

        program->add_section(parse_section(section_name));
    }

    return program;
}

shared_ptr<Ast::Section> Parser::parse_section(optional<string>& section_name) {
    PARSER_TRACE();
    auto section = make_shared<Ast::Section>();
    section->name = std::move(section_name);

    do {
        optional<string> block_name;
        if (match_start_of_block()) {
            block_name = parse_start_of_block();
        }
        auto block = section->get_or_create_block(block_name);
        parse_into_block(block);
    } while (!eof() && match_start_of_block());

    return section;
}

void Parser::parse_into_block(const shared_ptr<Ast::Block>& block) {
    PARSER_TRACE();

    while (!eof()) {
        while (is_empty_line()) skip_to_next_line();

        if (match_start_of_block() || match_start_of_section()) break;

        block->add_command(parse_command());
    }
}

bool Parser::is_empty_line() {
    PARSER_TRACE();
    size_t lookahead = 0;

    while(!eof(lookahead) && peek_type(lookahead) != Token::NewLine) {
        auto type = peek_type(lookahead);
        if (type != Token::Whitespace && type != Token::Comment) {
            return false;
        }

        lookahead++;
    }
    return true;
}

void Parser::skip_to_next_line() {
    PARSER_TRACE();
    while(!eof() && peek_type() != Token::NewLine) {
        index++;
    }
    if (!eof()) index++;
}

shared_ptr<Ast::Command> Parser::parse_command() {
    PARSER_TRACE();
    optional<string> command_label;
    if (peek_type() == Token::Label) {
        command_label = peek().content;
        index++;
    }

    expect(Token::Whitespace);

    bool extended = peek_type() == Token::Plus;
    if (extended) index++;

    auto opcode = get_instruction_mnemonic(peek_type());

    if (opcode) {
        index++;
        return parse_instruction(command_label, extended, opcode);
    }

    if (extended) {
        unexpected(peek());
    }

    auto directive = get_directive_mnemonic(peek_type());

    if (directive) {
        index++;
        return parse_directive(command_label, directive);
    }

    unexpected(peek());
}

bool Parser::match_start_of_block() {
    PARSER_TRACE();
    return match_token_after_label_or_whitespace(Token::USE);
}

bool Parser::match_start_of_section() {
    PARSER_TRACE();
    return match_token_after_label_or_whitespace(Token::CSECT);
}

// match [label](whitespace)(type)
bool Parser::match_token_after_label_or_whitespace(Token::Type type) {
    if (eof()) return false;

    size_t lookahead = 0;
    if (peek_type(0) == Token::Label) lookahead++;

    if (eof(lookahead) || peek_type(lookahead) != Token::Whitespace) return false;

    lookahead++;

    return !eof(lookahead) && peek_type(lookahead) == type;
}

// parse [label](whitespace)(CSECT)(parse comment)
optional<string> Parser::parse_start_of_section() {
    PARSER_TRACE();
    optional<string> section_name;
    if (peek_type() == Token::Label) {
        section_name = peek().content;
        index++;
    }

    expect(Token::Whitespace);
    expect(Token::CSECT);

    parse_comment();

    return section_name;
}

void Parser::error(string_view what) {
    std::cerr << "Parser error: " << what << std::endl;
    __asm__ volatile("int $0x03");
    exit(1);
}

void Parser::unexpected(const Token& token, Token::Type expected) {
    stringstream ss;
    ss << "Token::" << Token::type_to_str(expected);
    unexpected(token, ss.view());
}

void Parser::unexpected(Token const& token, std::string_view expected) {
    stringstream ss {};
    ss << "unexpected Token::" << Token::type_to_str(token.type) << " [" << token.content << "] at line " << token.line;
    if (!expected.empty()) {
        ss << " expected [" << expected << ']';
    }

    error(ss.view());
}

// parse [.comment](newline)
optional<string> Parser::parse_comment() {
    PARSER_TRACE();
    skip_if_whitespace();

    if (eof()) return std::nullopt;

    optional<string> maybe_comment;
    if (peek_type() == Token::Comment) {
        maybe_comment = peek().content;
        index++;
    }

    if (eof()) return maybe_comment;

    expect(Token::NewLine);

    return maybe_comment;
}

optional<string> Parser::parse_start_of_block() {
    PARSER_TRACE();
    expect(Token::Whitespace);
    expect(Token::USE);

    optional<string> block_name;

    if (peek_type() == Token::Whitespace) {
        index++;

        expect(Token::Label, false);
        block_name = get().content;
    }

    parse_comment();

    return block_name;
}

void Parser::expect(Token::Type type, bool advance) {
    if (eof()) error("Unexpected end of file");
    if (peek_type() != type) unexpected(peek(), type);
    if (advance) index++;
}

shared_ptr<Ast::Instruction>
Parser::parse_instruction(optional<string> label, bool extended, InstructionMnemonic const* mnemonic) {
    PARSER_TRACE();
    auto instruction = make_shared<Ast::Instruction>();
    instruction->mnemonic = mnemonic;
    instruction->label = std::move(label);
    if (extended) {
        instruction->flags.set_ni(0b11);
        instruction->flags.set_extended(true);
    }

    auto f = mnemonic->format;

    switch (f) {
        case Format::F3:
        case Format::F1:
            break;
        case Format::F2_num: {
            expect(Token::Whitespace);
            auto num = parse_number();

            instruction->operand = OperandsNum { static_cast<uint8_t>(num) };
            break;
        }

        case Format::F2_reg:
        case Format::F2_reg_num:
        case Format::F2_reg_reg: {
            expect(Token::Whitespace);
            auto reg_1 = parse_register();

            if (f == Format::F2_reg) {
                instruction->operand = OperandsReg {reg_1 };
                break;
            }

            skip_if_whitespace();
            expect(Token::Comma);
            skip_if_whitespace();

            if (f == Format::F2_reg_num) {
                auto num = parse_number();
                instruction->operand = OperandsRegNum {reg_1, static_cast<uint8_t>(num) };
            } else {
                auto reg_2 = parse_register();
                instruction->operand = OperandsRegReg {reg_1, reg_2 };
            }

            break;
        }
        case Format::F3_4_mem: {
            expect(Token::Whitespace);
            auto expr = parse_expression();
            instruction->operand = OperandsExpr{std::move(expr) };
            skip_if_whitespace();
            if (!eof() && peek_type() == Token::Comma) {
                index++;
                skip_if_whitespace();
                expect(Token::X);

                instruction->flags.set_indexed(true);
            }
            break;
        }
        default:
            error("Unexpected instruction format");
    }

    instruction->comment = parse_comment();

    return instruction;
}

int Parser::parse_number() {
    PARSER_TRACE();
    bool negative = peek_type() == Token::Minus;
    if (negative) index++;

    size_t start_index = negative;

    switch (peek_type()) {
        case Token::NumberDec:
            break;
        case Token::NumberHex:
        case Token::NumberBin:
        case Token::NumberOct:
            start_index += 2;
            break;
        default:
            unexpected(peek(), "Number");
    }

    Token number = get();
    Radix radix = number.to_radix();

    if (radix == Radix::Invalid) error("Invalid radix");

    try {
        auto num = std::stoi(number.content, &start_index, static_cast<int>(radix));
        return negative ? -num : num;
    } catch (std::invalid_argument&) {
        error("Couldn't parse number");
    }
}

Register Parser::parse_register() {
    PARSER_TRACE();
    switch (get_type()) {
        case Token::A: return Register::A;
        case Token::X: return Register::X;
        case Token::L: return Register::L;
        case Token::B: return Register::B;
        case Token::S: return Register::S;
        case Token::T: return Register::T;
        case Token::F: return Register::F;
        default:
            unexpected(peek(-1), "Register");
    }
}

void Parser::skip_if_whitespace() {
    PARSER_TRACE();
    while (!eof() && peek_type() == Token::Whitespace) index++;
}

shared_ptr<Ast::Directive> Parser::parse_directive(optional<string> label, DirectiveMnemonic const* mnemonic) {
    PARSER_TRACE();
    auto directive = make_shared<Ast::Directive>();

    directive->label = std::move(label);
    directive->mnemonic = mnemonic;

    auto f = mnemonic->format;
    switch (f) {
        case Format::D:
            break;
        case Format::D_expr: {
            expect(Token::Whitespace);
            directive->operand = OperandsExpr { parse_expression() };
            break;
        }
        case Format::D_sym: {
            expect(Token::Whitespace);
            expect(Token::Label, false);

            directive->operand = OperandsSymbol { get().content };
            break;
        }
        case Format::D_sym_array: {
            expect(Token::Whitespace);

            vector<string> symbols;
            bool expect_more = true;

            while (!eof() && expect_more) {
                expect(Token::Label, false);
                symbols.emplace_back(get().content);

                skip_if_whitespace();

                if (peek_type() == Token::Comma) {
                    index++;
                    skip_if_whitespace();
                    continue;
                } else {
                    expect_more = false;
                }
            }

            directive->operand = OperandsSymbolArray { std::move(symbols) };
            break;
        }
        case Format::S_reserve: {
            expect(Token::Whitespace);
            directive->operand = OperandsExpr { parse_expression() };
            break;
        }
        case Format::S_initialize: {
            expect(Token::Whitespace);
            switch (peek_type()) {
                case Token::CharReservation:
                case Token::HexReservation: {
                    directive->operand = OperandsByteArray { parse_reservation() };
                    break;
                }
                default:
                    directive->operand = OperandsExpr { parse_expression() };
            }
            break;
        }
        default:
            error("Unexpected directive format");
    }

    directive->comment = parse_comment();

    return directive;
}

// [label][plus]([3]-[2])
shared_ptr<Ast::Expression> Parser::parse_expression() {
    PARSER_TRACE();
    size_t start = index;

    bool finish = false;
    while (!eof() && !finish) {
        auto t = peek_type();
        switch (t) {
            case Token::Immediate:
            case Token::Indirect:
            case Token::Literal:

            case Token::NumberBin:
            case Token::NumberOct:
            case Token::NumberDec:
            case Token::NumberHex:
            case Token::Minus:
            case Token::Plus:
            case Token::Asterisk:
            case Token::Slash:

            case Token::OpenParenthesis:
            case Token::CloseParenthesis:

            case Token::Whitespace:
            case Token::Label:
                index++;
                break;
            default:
                finish = true;
                break;
        }
    }
    size_t end = index;

    if (start == end) unexpected(peek(), "Expression");
    return build_expression_tree(start, end, true);
}

shared_ptr<Ast::Expression> Parser::build_expression_tree(size_t start, size_t end, bool can_be_unary) { // NOLINT(misc-no-recursion)
    PARSER_TRACE();
    while (at_type(start) == Token::Whitespace) start++;
    while (at_type(end - 1) == Token::Whitespace) end--;

    if (end - start <= 0) error("Building expression tree failed");

    if (end - start == 1) {
        return parse_number_or_symbol(start);
    }

    auto is_binary_operand = [](Token::Type t) {
        switch (t) {
            case Token::Immediate:
            case Token::Indirect:
            case Token::Literal:
            case Token::NumberBin:
            case Token::NumberOct:
            case Token::NumberDec:
            case Token::NumberHex:
            case Token::Asterisk:
            case Token::Label:
                return true;
            default:
                return false;
        }
    };

    // (* * 2)
    // 3 + *
    // 1 * 2
    // * * var
    // var * var
    // * * *
    // *

    auto is_multiplication = [&](size_t i) {
        // Check lhs
        size_t lhs = i - 1;
        while (lhs >= start && at_type(lhs) == Token::Whitespace) lhs--;
        if (lhs < start || !is_binary_operand(at_type(lhs))) return false;

        // Check rhs
        size_t rhs = i + 1;
        while (rhs < end && at_type(rhs) == Token::Whitespace) rhs++;
        if (rhs >= end || !is_binary_operand(at_type(rhs))) return false;

        return true;
    };

    size_t plus_minus = -1;
    size_t mul_div = -1;
    size_t unary = -1;
    int paren_count = 0;

    for (size_t i = start; i < end; i++) {
        auto type = at_type(i);

        if (type == Token::OpenParenthesis) paren_count++;
        else if (type == Token::CloseParenthesis) paren_count--;
        else if (paren_count == 0) {
            switch (type) {
                case Token::Immediate:
                case Token::Indirect:
                case Token::Literal:
                    if (unary != -1) {
                        unexpected(at(i), "Cannot have multiple unary operators");
                    }
                    unary = i;
                    break;
                case Token::Minus:
                case Token::Plus:
                    plus_minus = i;
                    break;
                case Token::Asterisk:
                    if (is_multiplication(i)) {
                        mul_div = i;
                    }
                    break;
                case Token::Slash:
                    mul_div = i;
                    break;
                default:
                    break;
            }
        } else if (paren_count < 0) {
            unexpected(at(i));
        }
    }

    if (paren_count != 0) {
        unexpected(at(end), "Closing bracket");
    }

    if (unary != -1) {
        if (!can_be_unary) {
            unexpected(at(unary), "Unary operator can only be top-level");
        }

        auto expr = make_shared<Ast::UnaryExpression>();
        using enum Ast::UnaryExpression::Operation;

        switch (at_type(unary)) {
            case Token::Immediate: expr->operation = Immediate; break;
            case Token::Indirect: expr->operation = Indirect; break;
            case Token::Literal: expr->operation = Literal; break;
            default:
                unexpected(at(unary), "Unary operator");
        }

        if (unary != start) {
            unexpected(at(unary), "Operator can only be at start of expression");
        }
        expr->expression = build_expression_tree(start + 1, end, false);
        return expr;
    }

    if (plus_minus != -1 || mul_div != -1) {
        size_t operation_index = (plus_minus != -1) ? plus_minus : mul_div;

        auto expr = make_shared<Ast::BinaryExpression>();

        using enum Ast::BinaryExpression::Operation;

        switch (at_type(operation_index)) {
            case Token::Minus: expr->operation = Subtraction; break;
            case Token::Plus: expr->operation = Addition; break;
            case Token::Asterisk: expr->operation = Multiplication; break;
            case Token::Slash: expr->operation = Division; break;
            default:
                unexpected(at(unary), "Binary operator");
        }

        expr->lhs = build_expression_tree(start, operation_index, false);
        expr->rhs = build_expression_tree(operation_index + 1, end, false);
        return expr;
    }

    return build_expression_tree(start + 1, end - 1, can_be_unary);
}

shared_ptr<Ast::Expression> Parser::parse_number_or_symbol(size_t start) {
    PARSER_TRACE();
    if (at_type(start) == Token::Label || at_type(start) == Token::Asterisk) {
        auto expr = make_shared<Ast::SymbolExpression>();
        expr->symbol_name = at(start).content;
        return expr;
    } else {
        auto expr = make_shared<Ast::NumericExpression>();
        size_t old_index = index;
        index = start;
        expr->value = parse_number();
        index = old_index;
        return expr;
    }
    unexpected(at(start), "Number or symbol_name");
}

vector<Byte_t> Parser::parse_reservation() {
    vector<Byte_t> reserved_bytes;

    auto parse_char_reservation = [&](string_view content) {
        for (size_t i = 2; i < content.size() - 1; i++) {
            reserved_bytes.push_back(content[i]);
        }
    };

    auto parse_hex_reservation = [&](string_view content) {
        for (size_t i = 2; i < content.size() - 1; i+=2) {
            if (i + 1 >= content.size() - 1) unexpected(peek(), "Unaligned hex reservation");

            try {
                auto str_byte = string {content.substr(i, 2)};
                auto byte = std::stoi(str_byte, nullptr, 16);
                reserved_bytes.push_back(byte);
            } catch (std::invalid_argument&) {
                unexpected(peek(), "Invalid hex byte");
            }
        }
    };


    switch (peek_type()) {
        case Token::CharReservation:
            parse_char_reservation(peek().content);
            break;
        case Token::HexReservation:
            parse_hex_reservation(peek().content);
            break;
        default:
            unexpected(peek(), "Reservation");
    }
    index++;
    return reserved_bytes;
}
