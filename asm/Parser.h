//
// Created by Lenart on 12/11/2022.
//

#ifndef ASS2_PARSER_H
#define ASS2_PARSER_H


#include <vector>
#include "Lexer.h"
#include "ast/SicAST.h"
#include "../common/Mnemonics.h"

using std::shared_ptr;
using std::string;
using std::string_view;
using std::optional;
using std::vector;

class Parser {
public:
    explicit Parser(vector<Token> const& input_data) :
            m_input(input_data) {}

    shared_ptr<Ast::Program> parse();


private:
    [[nodiscard]] bool eof(size_t steps = 0) const;
    [[nodiscard]] Token peek(size_t steps = 0) const;
    [[nodiscard]] Token::Type peek_type(size_t steps = 0) const;
    [[nodiscard]] Token at(size_t index) const;
    [[nodiscard]] Token::Type at_type(size_t index) const;
    Token get();
    Token::Type get_type();


private:
    shared_ptr<Ast::Program> parse_program();
    shared_ptr<Ast::Section> parse_section(optional<string>& section_name);
    void parse_into_block(const shared_ptr<Ast::Block>& block);
    shared_ptr<Ast::Command> parse_command();
    shared_ptr<Ast::Instruction>
    parse_instruction(optional<string> label, bool extended, InstructionMnemonic const* mnemonic);
    shared_ptr<Ast::Directive>
    parse_directive(optional<string> label, DirectiveMnemonic const* mnemonic);
    vector<Byte_t> parse_reservation();
    shared_ptr<Ast::Expression> parse_expression();
    shared_ptr<Ast::Expression> build_expression_tree(size_t start, size_t end, bool can_be_unary);
    shared_ptr<Ast::Expression> parse_number_or_symbol(size_t start);

    int parse_number();
    Register parse_register();

    optional<string> parse_start_of_section();
    optional<string> parse_start_of_block();
    optional<string> parse_comment();

    void expect(Token::Type type, bool advance = true);

    bool is_empty_line();
    void skip_if_whitespace();
    void skip_to_next_line();

    bool match_start_of_block();
    bool match_start_of_section();

    bool match_token_after_label_or_whitespace(Token::Type type);

    [[noreturn]] static void error(string_view what) ;
    [[noreturn]] static void unexpected(const Token& token, Token::Type expected = Token::Invalid) ;
    [[noreturn]] static void unexpected(const Token& token, std::string_view expected) ;
private:
    size_t index = 0;
    vector<Token> const& m_input;
    const bool debug_print { false };
};


#endif //ASS2_PARSER_H
