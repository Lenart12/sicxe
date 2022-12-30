//
// Created by Lenart on 12/11/2022.
//

#include <functional>
#include <variant>
#include <iostream>
#include "SicAST.h"
#include "Visitor.h"


namespace Ast {

using std::stringstream;
using std::make_shared;
using std::holds_alternative;
using std::get;

void Program::accept(Visitor& visitor) {
    visitor.visit(this, NodeType::Program);
    for (auto& section : m_sections) {
        section->accept(visitor);
    }
    visitor.leave(this, NodeType::Program);
}

void Block::accept(Visitor &visitor) {
    visitor.visit(this, NodeType::Block);
    for (auto& command : commands) {
        command->accept(visitor);
    }
    visitor.leave(this, NodeType::Block);
}

void Section::accept(Visitor &visitor) {
    visitor.visit(this, NodeType::Section);
    for (auto& block : blocks) {
        block->accept(visitor);
    }
    visitor.leave(this, NodeType::Section);
}

void Instruction::accept(Visitor &visitor) {
    visitor.visit(this, NodeType::Instruction);
    if (holds_alternative<OperandsExpr>(operand)) {
        get<OperandsExpr>(operand).expression->accept(visitor);
    }
    visitor.leave(this, NodeType::Instruction);
}

void Command::accept(Visitor &visitor) {
    visitor.visit(this, NodeType::Command);
    visitor.leave(this, NodeType::Command);
}

void Directive::accept(Visitor &visitor) {
    visitor.visit(this, NodeType::Directive);
    if (holds_alternative<OperandsExpr>(operand)) {
        get<OperandsExpr>(operand).expression->accept(visitor);
    }
    visitor.leave(this, NodeType::Directive);
}

void Expression::accept(Visitor& visitor) {
    visitor.visit(this, NodeType::Expression);
    visitor.leave(this, NodeType::Expression);
}

void SymbolExpression::accept(Visitor& visitor) {
    visitor.visit(this, NodeType::SymbolExpression);
    visitor.leave(this, NodeType::SymbolExpression);
}

void NumericExpression::accept(Visitor& visitor) {
    visitor.visit(this, NodeType::NumericExpression);
    visitor.leave(this, NodeType::NumericExpression);
}

void BinaryExpression::accept(Visitor& visitor) {
    visitor.visit(this, NodeType::BinaryExpression);
    lhs->accept(visitor);
    rhs->accept(visitor);
    visitor.leave(this, NodeType::BinaryExpression);
}

void UnaryExpression::accept(Visitor& visitor) {
    visitor.visit(this, NodeType::UnaryExpression);
    expression->accept(visitor);
    visitor.leave(this, NodeType::UnaryExpression);
}

ostream &operator<<(ostream &os, const Node &node) {
    node.print(os);
    return os;
}

bool Program::contains_section(const optional<string>& _name) {
    return std::any_of(m_sections.begin(), m_sections.end(),
        [&](auto const& section) {
        return section->name == _name;
    });
}

void Program::add_section(shared_ptr<Section>&& section) {
    m_sections.push_back(std::move(section));
}

shared_ptr<Block> Section::get_or_create_block(const optional<string>& new_name) {
    for (auto const& block : blocks) {
        if (block->name == new_name) return block;
    }

    auto new_block = blocks.emplace_back(std::make_shared<Block>());
    new_block->name = new_name;

    return new_block;
}

void Block::add_command(shared_ptr<Command>&& command) {
    commands.push_back(std::move(command));
}

size_t Instruction::get_size() const {
    if (!mnemonic) return 0;
    switch (mnemonic->format) {
        case Format::F1: return 1;
        case Format::F2_num:
        case Format::F2_reg:
        case Format::F2_reg_num:
        case Format::F2_reg_reg: return 2;
        case Format::F3: return 3;
        case Format::F3_4_mem: return 3 + flags.is_extended();
        default:
            assert(!"Unknown format");
            abort();
    }
}

string property_to_string(const char* name, std::string const& value) {
    stringstream ss {};
    ss << name << " = " << value;
    return ss.str();
}

void Node::print(ostream& ostream) const {
    ostream << "Node [" << (void*) this << "]";
}

void Block::print(ostream& ostream) const {
    ostream << "Block [" << property_to_string("name", name.value_or("[default]"));
    ostream << ", " << property_to_string("commands", std::to_string(commands.size())) << "]";
}

void Program::print(ostream& ostream) const {
    ostream << "Program [" << property_to_string("sections", std::to_string(m_sections.size()))  << "]";
}

void Section::print(ostream& ostream) const {
    ostream << "Section [" << property_to_string("name", name.value_or("[default]"));
    ostream << ", " << property_to_string("blocks", std::to_string(blocks.size())) << "]";
}

bool Section::try_resolve_expr(Expression* expr) { // NOLINT(misc-no-recursion)
    if (expr->resolved_value.has_value()) return true;

    switch (expr->get_type()) {
        case NodeType::UnaryExpression: {
            auto& node = *dynamic_cast<UnaryExpression*>(expr);
            bool resolved = try_resolve_expr(node.expression.get());
            if (resolved) {
                node.resolved_value = node.expression->resolved_value;
                return true;
            }
            break;
        }
        case NodeType::BinaryExpression: {
            auto& node = *dynamic_cast<BinaryExpression*>(expr);

            if (!try_resolve_expr(node.lhs.get()) || !try_resolve_expr(node.rhs.get())) return false;

            auto lhs_value = node.lhs->resolved_value.value();
            auto rhs_value = node.rhs->resolved_value.value();

            using enum BinaryExpression::Operation;
            switch (node.operation) {
                case Addition:
                    node.resolved_value = lhs_value + rhs_value;
                    break;
                case Subtraction:
                    node.resolved_value = lhs_value - rhs_value;
                    break;
                case Multiplication:
                    node.resolved_value = lhs_value * rhs_value;
                    break;
                case Division:
                    node.resolved_value = lhs_value / rhs_value;
                    break;
            }
            return true;
        }
        case NodeType::SymbolExpression: {
            auto& node = *dynamic_cast<SymbolExpression*>(expr);

            auto maybe_symbol = internal_symbols.find_symbol(node.symbol_name);

            if (!maybe_symbol.has_value()) {
                maybe_symbol = imported_symbols.find_symbol(node.symbol_name);
                node.imported = true;
            }

            if (!maybe_symbol.has_value()) {
                return false;
            }

            node.resolved_value = maybe_symbol.value().address;
            return true;
        }
        case NodeType::NumericExpression: {
            auto& node = *dynamic_cast<NumericExpression*>(expr);
            node.resolved_value = node.value;
            return true;
        }
        default:
            return false;
    }
    return false;
}


void Command::print(ostream& ostream) const {
    ostream << "Command [" ;
    ostream << property_to_string("location", std::to_string(location)) << ", ";
    bool has_comment = comment.has_value();
    if (label.has_value()) {
        ostream << property_to_string("label", label.value());

        if (has_comment) ostream << ", ";
    }

    if (has_comment) {
        ostream << property_to_string("comment", comment.value());
    }
    ostream << "]";
}

void Instruction::print(ostream& ostream) const {
    ostream << "Instruction [" ;
    ostream << property_to_string("location", std::to_string(location)) << ", ";
    bool has_comment = comment.has_value();
    if (label.has_value()) {
        ostream << property_to_string("label", label.value());
        ostream << ", ";
    }

    ostream << property_to_string("op", string{mnemonic->mnemonic});

    if (has_comment) {
        ostream << ", " << property_to_string("comment", comment.value());
    }
    ostream << "]";
}

size_t Command::get_size() const {
    return 0;
}

void Directive::print(ostream& ostream) const {
    ostream << "Directive [" ;
    ostream << property_to_string("location", std::to_string(location)) << ", ";
    bool has_comment = comment.has_value();
    if (label.has_value()) {
        ostream << property_to_string("label", label.value());
        ostream << ", ";
    }

    ostream << property_to_string("op", string{mnemonic->mnemonic});

    if (has_comment) {
        ostream << ", " << property_to_string("comment", comment.value());
    }
    ostream << "]";
}

size_t Directive::get_size() const {
    using enum ::Directive;
    switch (mnemonic->direcitve) {
        case WORD:
            return 3;
        case BYTE:
            if (holds_alternative<OperandsExpr>(operand)) {
                return 1;
            }
            else if (holds_alternative<OperandsByteArray>(operand)) {
                return get<OperandsByteArray>(operand).bytes.size();
            }
            return -1;
        case RESW:
        case RESB: {
            if (!holds_alternative<OperandsExpr>(operand)) {
                return -1;
            }
            auto& op = get<OperandsExpr>(operand);

            if (!op.expression->resolved_value.has_value()) {
                return -1;
            }

            auto element_size = mnemonic->direcitve == RESB ? 1 : 3;
            return op.expression->resolved_value.value() * element_size;
        }
        default:
            break;
    }

    return 0;
}

void Expression::print(ostream& ostream) const {
    ostream << "Expression [";
    if (resolved_value.has_value())
        ostream << ", "
        << property_to_string("resolved_value", std::to_string(resolved_value.value()));
    ostream <<"]";
}

void UnaryExpression::print(ostream& ostream) const {
    ostream << "UnaryExpression [operation = ";
    switch (operation) {
        case Operation::Literal:
            ostream << "Literal";
            break;
        case Operation::Indirect:
            ostream << "Indirect";
            break;
        case Operation::Immediate:
            ostream << "Immediate";
            break;
    }
    if (resolved_value.has_value())
        ostream << ", "
                << property_to_string("resolved_value", std::to_string(resolved_value.value()));
    ostream << "]";
}



void BinaryExpression::print(ostream& ostream) const {
    ostream << "BinaryExpression [operation = ";
    switch (operation) {
        case Operation::Addition:
            ostream << "Addition";
            break;
        case Operation::Subtraction:
            ostream << "Subtraction";
            break;
        case Operation::Multiplication:
            ostream << "Multiplication";
            break;
        case Operation::Division:
            ostream << "Division";
            break;
    }
    if (resolved_value.has_value())
        ostream << ", "
                << property_to_string("resolved_value", std::to_string(resolved_value.value()));
    ostream << "]";
}

bool BinaryExpression::try_resolve() {
    if (lhs->resolved_value.has_value() && rhs->resolved_value.has_value()) {
        auto lhs_value = lhs->resolved_value.value();
        auto rhs_value = rhs->resolved_value.value();

        using enum Operation;
        switch (operation) {
            case Addition:
                resolved_value = lhs_value + rhs_value;
                break;
            case Subtraction:
                resolved_value = lhs_value - rhs_value;
                break;
            case Multiplication:
                resolved_value = lhs_value * rhs_value;
                break;
            case Division:
                resolved_value = lhs_value / rhs_value;
                break;
        }
        return true;
    }
    return false;
}

void SymbolExpression::print(ostream& ostream) const {
    ostream << "SymbolExpression [" << property_to_string("symbol_name", symbol_name);
    if (resolved_value.has_value())
        ostream << ", "
                << property_to_string("resolved_value", std::to_string(resolved_value.value()));
    ostream << "]";
}


void NumericExpression::print(ostream& ostream) const {
    ostream << "NumericExpression [" << property_to_string("value", std::to_string(value));
    if (resolved_value.has_value())
        ostream << ", "
                << property_to_string("resolved_value", std::to_string(resolved_value.value()));
    ostream << "]";
}



string UnaryExpression::to_string() {
    switch (operation) {
        case Operation::Literal:
            return "=" + expression->to_string();
        case Operation::Indirect:
            return "@" + expression->to_string();
        case Operation::Immediate:
            return "#" + expression->to_string();
    }
    return {};
}

bool UnaryExpression::is_imported() {
    return expression->is_imported();
}

bool UnaryExpression::is_absolute() {
    return expression->is_absolute();
}

string BinaryExpression::to_string() {
    stringstream ss;
    ss << '(' << lhs->to_string();
    switch (operation) {
        case Operation::Addition:
            ss << " + ";
            break;
        case Operation::Subtraction:
            ss << " - ";
            break;
        case Operation::Multiplication:
            ss << " * ";
            break;
        case Operation::Division:
            ss << " / ";
            break;
    }
    ss << rhs->to_string() << ')';
    return ss.str();
}

bool BinaryExpression::is_imported() {
    if (lhs->is_imported()) return true;
    return rhs->is_imported();
}

bool BinaryExpression::is_absolute() {
    return lhs->is_absolute() && rhs->is_absolute();
}

string SymbolExpression::to_string() {
    return symbol_name;
}

bool SymbolExpression::is_imported() {
    return imported;
}

bool SymbolExpression::is_absolute() {
    return false;
}

string NumericExpression::to_string() {
    return std::to_string(value);
}

bool NumericExpression::is_imported() {
    return false;
}

bool NumericExpression::is_absolute() {
    return true;
}


}

