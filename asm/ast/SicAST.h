//
// Created by Lenart on 12/11/2022.
//

#ifndef ASS2_SICAST_H
#define ASS2_SICAST_H

#include <vector>
#include <cstdint>
#include <string>
#include <ostream>
#include <memory>
#include <optional>
#include <variant>
#include <map>

#include "Forward.h"
#include "../../common/SicTypes.h"
#include "../../common/Mnemonics.h"
#include "../../common/Flags.h"
#include "SymbolTable.h"

namespace Ast {

using std::vector;
using std::shared_ptr;
using std::optional;
using std::string;
using std::ostream;
using std::variant;
using std::map;

class Visitor;

#define AST_NODE(NodeName) \
public: \
    void accept(Visitor &visitor) override; \
    NodeType get_type() override { return NodeType::NodeName; } \
protected: \
    void print(ostream &ostream) const override; \
private:


class Node {
public:
    friend ostream &operator<<(ostream &os, const Node &node);
    virtual void accept(Visitor& visitor) = 0;
    virtual NodeType get_type() = 0;
protected:
    virtual void print(ostream& ostream) const;
};

class Command : public Node {
    AST_NODE(Command)

public:
    optional<string> label;
    optional<string> comment;
    Operand_t operand { OperandsNone {} };
    Address_t location {};

    [[nodiscard]] virtual size_t get_size() const;
};

class Instruction : public Command {
    AST_NODE(Instruction)

public:
    InstructionMnemonic const* mnemonic {};
    Flags flags;

    [[nodiscard]] size_t get_size() const override;
};

class Directive : public Command {
    AST_NODE(Directive)
public:
    DirectiveMnemonic const* mnemonic {};

    [[nodiscard]] size_t get_size() const override;
};

class Block : public Node {
    AST_NODE(Block)
public:
    optional<string> name;
    vector<shared_ptr<Command>> commands;

    void add_command(shared_ptr<Command>&& command);
};

class Section : public Node {
    AST_NODE(Section)
public:
    optional<string> name;
    Address_t locctr {};
    SymbolTable internal_symbols;
    SymbolTable exported_symbols;
    SymbolTable imported_symbols;
    map<string, shared_ptr<Expression>> equ_symbols;
    vector<shared_ptr<Block>> blocks;

    shared_ptr<Block> get_or_create_block(const optional<string>& name);
    bool try_resolve_expr(Expression* expr);
};

class Program : public Node {
    AST_NODE(Program)
public:
    optional<string> name;
    optional<Address_t> load_address;
    optional<Address_t> execution_start_address;

    bool contains_section(optional<string> const& name);
    void add_section(shared_ptr<Section>&& section);
private:
    vector<shared_ptr<Section>> m_sections;
};

class Expression : public Node {
    AST_NODE(Expression)
public:
    optional<int> resolved_value;
    virtual bool is_imported() { return false; };
    virtual bool is_absolute() { return true; };
    virtual string to_string() { return {}; };
};

class UnaryExpression : public Expression {
    AST_NODE(UnaryExpression)
public:
    enum class Operation {
        Literal,
        Indirect,
        Immediate
    } operation;
    shared_ptr<Expression> expression;

    bool is_imported() override;
    bool is_absolute() override;
    string to_string() override;
};

class BinaryExpression : public Expression {
    AST_NODE(BinaryExpression)
public:
    enum class Operation {
        Addition,
        Subtraction,
        Multiplication,
        Division
    } operation;

    shared_ptr<Expression> lhs;
    shared_ptr<Expression> rhs;

    bool try_resolve();
    bool is_imported() override;
    bool is_absolute() override;
    string to_string() override;
};

class NumericExpression : public Expression {
    AST_NODE(NumericExpression)
public:
    int value {0};

    bool is_imported() override;
    bool is_absolute() override;
    string to_string() override;
};

class SymbolExpression : public Expression {
    AST_NODE(SymbolExpression)

public:
    string symbol_name;
    bool imported {false};
    bool is_imported() override;
    bool is_absolute() override;
    string to_string() override;
};

}







#endif //ASS2_SICAST_H
