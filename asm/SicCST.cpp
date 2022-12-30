//
// Created by Lenart on 27/12/2022.
//

#include "SicCST.h"
#include "ast/SicAST.h"

#include <iostream>
#include <iomanip>
#include <ios>
#include <utility>

using std::cout;
using std::cerr;
using std::endl;
using std::holds_alternative;
using std::get;
using std::setw;
using std::setfill;
using std::left;
using std::right;
using std::hex;
using std::flush;
using std::stringstream;
using std::string_view;
using std::uppercase;
using std::nouppercase;
using std::nullopt;

#define to_node(NodeName) *dynamic_cast<Ast::NodeName*>(_node)

[[noreturn]] void error(Ast::Node* _node, string_view what) {
    cerr << "CST error on node " << *_node << ": " << what << endl;
    __asm__ volatile("int $0x03");
    exit(1);
}

ProgramAssembler::ProgramAssembler(shared_ptr<Ast::Program> program)
        : program(std::move(program))
{}

void ProgramAssembler::assemble_program() {

    AbsoluteExpressionResolver absolute_expression_resolver;
    program->accept(absolute_expression_resolver);

    DefineSymbolAndLocationVisitor define_symbol_and_location_visitor;
    program->accept(define_symbol_and_location_visitor);

    SymbolExpressionResolver symbol_expression_resolver;
    program->accept(symbol_expression_resolver);

    if (ast_stream) {
        AstTreeDump ast_tree_dump {*ast_stream};
        program->accept(ast_tree_dump);
    }

    if (lst_stream) {
        LstGenerator lst_generator {*lst_stream};
        program->accept(lst_generator);
    }

    if (obj_stream) {
        ProgramObjectGenerator program_object_generator {*obj_stream};
        program->accept(program_object_generator);
    }

}

AstTreeDump::AstTreeDump(ostream& _os) : os(_os) {}

void AstTreeDump::visit(Ast::Node* _node, Ast::NodeType type) {
    for (size_t i = 0; i < m_indent; i++) {
        os << "  ";
    }
    os << *_node << endl;
    m_indent++;
}

void AstTreeDump::leave(Ast::Node* _node, Ast::NodeType type) {
    m_indent--;
    if (type == Ast::NodeType::Program) os << flush;
}

void LstGenerator::visit(Ast::Node* _node, Ast::NodeType type) {
    switch (type) {
        case Ast::NodeType::Node:
            break;
        case Ast::NodeType::Block: {
            auto& node = to_node(Block);
            os << "----- Block " << node.name.value_or("<default>") << " -----" << endl;
            os << "Stats: ";
            os << "commands=" << node.commands.size() << endl;
            break;
        }
        case Ast::NodeType::Program: {
            auto& node = to_node(Program);
            os << "===== Program " << node.name.value_or("<default>") << " =====" << endl;
            os << "Stats: ";
            os << "execution start address=" << node.execution_start_address.value_or(0) << " ";
            os << "load address=" << node.load_address.value_or(0) << endl << endl;
            break;
        }
        case Ast::NodeType::Section: {
            auto& node = to_node(Section);
            os << "***** Section " << node.name.value_or("<default>") << " *****" << endl;
            os << "Stats: ";
            os << "size=" << node.locctr << " ";
            os << "Symbols:" << endl;
            os << "    name        address" << endl;
            for (auto const& symbol : node.internal_symbols.get_table()) {
                os << "    " << setw(8) << left << symbol.label << "    ";
                os << setw(8) << symbol.address;
                if (node.exported_symbols.find_symbol(symbol.label).has_value()) {
                    os << " exported";
                }
                os << endl;
            }
            for (auto const& symbol : node.imported_symbols.get_table()) {
                os << "    " << setw(8) << left << symbol.label << "    ";
                os << setw(8) << "imported" << endl;
            }
            break;
        }
        case Ast::NodeType::Instruction: {
            auto& node = to_node(Instruction);
            os << setw(8) << setfill('0') << right << node.location;
            os << setfill(' ') << "    ";
            os << setw(8) << left << node.label.value_or("");
            if (node.flags.is_extended()) os << "+";
            os << setw(8 - node.flags.is_extended()) << left << node.mnemonic->mnemonic;

            auto op = operand_to_str(node.operand);

            if (!op.empty()) {
                os << " " << op;
            }

            if (node.flags.is_indexed()) {
                os << ", X";
            } else if (node.flags.is_base_relative()) {
                os << ", B";
            }

            os << " " << node.comment.value_or("") << endl;
            break;
        }
        case Ast::NodeType::Directive: {
            auto& node = to_node(Directive);
            os << setw(8) << setfill('0') << right << node.location;
            os << setfill(' ') << "    ";
            os << setw(8) << left << node.label.value_or("");
            os << setw(8) << left << node.mnemonic->mnemonic;

            auto op = operand_to_str(node.operand);

            if (!op.empty()) {
                os << " " << op;
            }

            os << " " << node.comment.value_or("") << endl;
            break;
        }
        default:
            break;
    }
}

void LstGenerator::leave(Ast::Node* _node, Ast::NodeType type) {
    switch (type) {
        case Ast::NodeType::Block: {
            auto& node = to_node(Block);
            os << "----/ Block " << node.name.value_or("<default>") << " /----" << endl;
            break;
        }
        case Ast::NodeType::Section: {
            auto& node = to_node(Section);
            os << "****/ Section " << node.name.value_or("<default>") << " /****" << endl;
            break;
        }
        default:
            break;
    }
}

LstGenerator::LstGenerator(ostream& _os) : os(_os) {}


void AbsoluteExpressionResolver::visit(Ast::Node* _node, Ast::NodeType type) {}

void AbsoluteExpressionResolver::leave(Ast::Node* _node, Ast::NodeType type) {
    switch (type) {
        case Ast::NodeType::BinaryExpression: {
            auto& node = to_node(BinaryExpression);
            node.try_resolve();
            break;
        }
        case Ast::NodeType::NumericExpression: {
            auto& node = to_node(NumericExpression);
            node.resolved_value = node.value;
            break;
        }
        default:
            return;
    }
}

void DefineSymbolAndLocationVisitor::visit(Ast::Node* _node, Ast::NodeType type) {
    switch (type) {
        case Ast::NodeType::Program:
            program = &to_node(Program);
            break;
        case Ast::NodeType::Section:
            locctr = 0;
            section = &to_node(Section);
            break;
        case Ast::NodeType::Directive: {
            auto& node = to_node(Directive);

            switch (node.mnemonic->direcitve) {
                case Directive::START: {
                    if (locctr != 0) {
                        error(_node, "START must be at the first instruction of program");
                    }

                    if (!holds_alternative<OperandsExpr>(node.operand)) {
                        error(_node, "Expected OperandsExpr");
                    }
                    auto& op = get<OperandsExpr>(node.operand);

                    if (!section->try_resolve_expr(op.expression.get())) {
                        error(_node, "Cannot resolve START address");
                    }

                    if (program->load_address.has_value()) {
                        error(_node, "Duplicate START");
                    }

                    program->name = node.label;
                    program->load_address = op.expression->resolved_value.value();
                    locctr = program->load_address.value();
                    break;
                }
                case Directive::ORG: {
                    if (!holds_alternative<OperandsExpr>(node.operand)) {
                        error(_node, "Expected OperandsExpr");
                    }
                    auto& op = get<OperandsExpr>(node.operand);

                    if (!op.expression->resolved_value.has_value()) {
                        error(_node, "Changing ORG to unresolved value");
                    }

                    locctr = op.expression->resolved_value.value();
                    break;
                }
                case Directive::EXTDEF: {
                    if (!holds_alternative<OperandsSymbolArray>(node.operand)) {
                        error(_node, "Expected OperandsSymbolArray");
                    }
                    auto& op = get<OperandsSymbolArray>(node.operand);

                    for (auto const& to_export : op.symbols) {

                        auto maybe_defined = section->internal_symbols.find_symbol(to_export);

                        if (maybe_defined.has_value()) {
                            section->exported_symbols.define_symbol(maybe_defined.value());
                        }

                        symbols_to_export.insert(to_export);
                    }
                    break;
                }
                case Directive::EXTREF:{
                    if (!holds_alternative<OperandsSymbolArray>(node.operand)) {
                        error(_node, "Expected OperandsSymbolArray");
                    }
                    auto& op = get<OperandsSymbolArray>(node.operand);

                    for (auto const& to_import : op.symbols) {
                        section->imported_symbols.define_symbol({to_import, 0});
                    }
                    break;
                }
                case Directive::EQU: {
                    node.location = locctr;
                    if (!node.label.has_value())
                        return;

                    if (!holds_alternative<OperandsExpr>(node.operand)) {
                        error(_node, "Expected OperandsExpr");
                    }
                    auto& op = get<OperandsExpr>(node.operand);

                    if (section->equ_symbols.contains(node.label.value())) {
                        error(_node, "Duplicate symbol " + node.label.value());
                    }

                    section->equ_symbols[node.label.value()] = op.expression;
                    return;
                }
                default:
                    break;
            }

            try_define_command(&node);
            break;
        }
        case Ast::NodeType::Instruction: {
            auto& node = to_node(Instruction);

            if (holds_alternative<OperandsExpr>(node.operand)) {
                auto& op = get<OperandsExpr>(node.operand);

                if (op.expression->get_type() == Ast::NodeType::UnaryExpression) {
                    if (node.flags.is_indexed()) {
                        error(_node, "Indexed addressing cannot be used here");
                    }
                    auto& unary = *dynamic_cast<Ast::UnaryExpression*>(op.expression.get());
                    node.flags.set_ni(0);
                    switch(unary.operation) {
                        // TODO
                        case Ast::UnaryExpression::Operation::Literal:
                            error(&unary, "Literal expression not supported");
                        case Ast::UnaryExpression::Operation::Indirect:
                            node.flags.set_indirect(true);
                            break;
                        case Ast::UnaryExpression::Operation::Immediate:
                            node.flags.set_immediate(true);
                            break;
                    }
                } else {
                    node.flags.set_ni(0b11);
                }
            } else {
                node.flags.set_ni(0b11);
            }

            try_define_command(&node);
            break;
        }
        case Ast::NodeType::SymbolExpression: {
            auto& node = to_node(SymbolExpression);

            if (node.symbol_name == "*") {
                node.resolved_value = locctr;
            }
            break;
        }
        default:
            return;
    }
}

void DefineSymbolAndLocationVisitor::leave(Ast::Node* _node, Ast::NodeType type) {
    switch (type) {
        case Ast::NodeType::Section: {
            section->locctr = locctr;
            bool try_resolving = true;
            while(try_resolving && !section->equ_symbols.empty()) {
                try_resolving = false;
                vector<SymbolTable::Symbol> resolved_symbols;
                for (auto const& [label, expr] : section->equ_symbols) {
                    if (section->try_resolve_expr(expr.get())) {
                        try_resolving = true;

                        resolved_symbols.push_back({label, static_cast<Address_t>(expr->resolved_value.value())});
                    }
                }

                for (auto const& symbol : resolved_symbols) {
                    try_define_symbol(_node, symbol);
                    section->equ_symbols.erase(symbol.label);
                }
            }


            if (!section->equ_symbols.empty()) {
                stringstream ss;
                ss << "Cant resolve EQU symbols:";
                for (auto const& symbol : section->equ_symbols) {
                    ss << " " << symbol.first;
                }
                error(_node, ss.str());
            }

            if (!symbols_to_export.empty()) {
                stringstream ss;
                ss << "Missing symbols to export:";
                for (auto const& symbol : symbols_to_export) {
                    ss << " " << symbol;
                }
                error(_node, ss.str());
            }

            break;
        }
        case Ast::NodeType::Directive:
        case Ast::NodeType::Instruction:
        case Ast::NodeType::Command: {
            auto& node = to_node(Command);

            auto command_size = node.get_size();

            if (command_size == -1) {
                error(_node, "Can not resolve expression");
            }
            locctr += command_size;
            break;
        }
        default:
            break;
    }
}

void DefineSymbolAndLocationVisitor::try_define_symbol(Ast::Node* _node, SymbolTable::Symbol const& symbol) {
    if (section->internal_symbols.find_symbol(symbol.label).has_value()) {
        error(_node, string { "Duplicate symbol " } + symbol.label);
    }

    section->internal_symbols.define_symbol(symbol);

    if (symbols_to_export.contains(symbol.label)) {
        section->exported_symbols.define_symbol(symbol);
        symbols_to_export.erase(symbol.label);
    }
}

void DefineSymbolAndLocationVisitor::try_define_command(Ast::Command* command) {
    auto& node = *command;

    node.location = locctr;

    if (node.label.has_value()) {
        auto label = node.label.value();
        try_define_symbol(command, {label, locctr});
    }
}

void SymbolExpressionResolver::visit(Ast::Node* _node, Ast::NodeType type) {
    switch (type) {
        case Ast::NodeType::Program:
            program = &to_node(Program);
            break;
        case Ast::NodeType::Section:
            section = &to_node(Section);
            break;
        case Ast::NodeType::Directive: {
            auto& node = to_node(Directive);

            switch(node.mnemonic->direcitve) {
                case Directive::END:{
                    if (!holds_alternative<OperandsExpr>(node.operand)) {
                        error(_node, "Expected OperandsExpr");
                    }
                    auto& op = get<OperandsExpr>(node.operand);

                    if (!section->try_resolve_expr(op.expression.get())) {
                        error(_node, "Cannot resolve END address");
                    }

                    if (program->execution_start_address.has_value()) {
                        error(_node, "Duplicate END");
                    }

                    program->execution_start_address = op.expression->resolved_value.value();
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case Ast::NodeType::NumericExpression:
        case Ast::NodeType::UnaryExpression:
        case Ast::NodeType::BinaryExpression:
        case Ast::NodeType::SymbolExpression: {
            auto& node = to_node(Expression);

            if (node.resolved_value.has_value())
                break;

            if (!section->try_resolve_expr(&node)) {
                error(_node, "Could not resolve expression");
            }
            break;
        }
        default:
            break;
    }
}

void SymbolExpressionResolver::leave(Ast::Node* _node, Ast::NodeType type) {}

void print_label(ostream& os, string const& label) {
    os << left << setfill(' ') << setw(6) << nouppercase << label;
}
template<typename T>
void print_hex(ostream& os, T value, int width) {
    os << right << setfill('0') << setw(width) << uppercase << hex << (int)value;
}


void ProgramObjectGenerator::visit(Ast::Node* _node, Ast::NodeType type) {
    switch (type) {
        case Ast::NodeType::Program: {
            program = &to_node(Program);
            break;
        }
        case Ast::NodeType::Section: {
            auto& section = to_node(Section);
            os << "H";
            print_label(os, section.name.value_or(program->name.value_or("null")));
            print_hex(os, section.name.has_value() ? 0 : program->load_address.value_or(0), 6);
            print_hex(os, section.locctr, 6);
            os << endl;

            if (!section.exported_symbols.get_table().empty()) {
                os << 'D';
                for (auto const& symbol : section.exported_symbols.get_table()) {
                    print_label(os, symbol.label);
                    print_hex(os, symbol.address, 6);
                }
                os << endl;
            }
            if (!section.imported_symbols.get_table().empty()) {
                os << 'R';
                for (auto const& symbol : section.imported_symbols.get_table()) {
                    print_label(os, symbol.label);
                }
                os << endl;
            }
            break;
        }
        case Ast::NodeType::Directive: {
            auto& node = to_node(Directive);

            switch (node.mnemonic->direcitve) {
                case Directive::BYTE:
                case Directive::WORD: {
                    if (holds_alternative<OperandsByteArray>(node.operand)) {
                        add_bytes(get<OperandsByteArray>(node.operand).bytes, node.location);
                    }
                    else if (holds_alternative<OperandsExpr>(node.operand)) {
                        vector<Byte_t> bytes;

                        auto& op = get<OperandsExpr>(node.operand);

                        if (op.expression->get_type() == Ast::NodeType::UnaryExpression) {
                            error(_node, "Cannot reserve unary expression");
                        }

                        if (op.expression->is_imported()) {
                            error(_node, "Cannot reserve imported expression");
                        }

                        auto value = static_cast<unsigned>(op.expression->resolved_value.value());

                        if (node.mnemonic->direcitve == Directive::BYTE) {
                            if (value > 0xff) {
                                error(_node, "Value is too big to reserve");
                            }
                            bytes.push_back(value);
                        } else {
                            if (value > 0xffffff) {
                                error(_node, "Value is too big to reserve");
                            }
                            bytes.reserve(3);
                            bytes.push_back((value & 0xff0000) >> 16);
                            bytes.push_back((value & 0x00ff00) >> 8);
                            bytes.push_back(value & 0x0000ff);
                        }

                        add_bytes(bytes, node.location);
                    } else {
                        error(_node, "Expected reservation operand but got " + operand_to_str(node.operand));
                    }
                    break;
                }
                case Directive::BASE: {
                    if (!holds_alternative<OperandsExpr>(node.operand)) {
                        error(_node, "Expected expression operand");
                    }
                    auto& expr = *get<OperandsExpr>(node.operand).expression;

                    if (!expr.resolved_value.has_value()) {
                        error(&expr, "Unresolved value");
                    }
                    base_register = expr.resolved_value.value();
                    break;
                }
                case Directive::NOBASE: {
                    base_register = nullopt;
                    break;
                }
                default:
                    break;
            }


            break;
        }
        case Ast::NodeType::Instruction: {
            auto& node = to_node(Instruction);

            vector<Byte_t> bytes;
            auto f = node.mnemonic->format;

            auto to_bytes_f2 = [&](Byte_t left_nibble, Byte_t right_nibble) {
                bytes.reserve(2);
                auto b1 = static_cast<Byte_t>(node.mnemonic->opcode);
                bytes.push_back(b1);

                Byte_t b2 = ((left_nibble << 4) & 0xf0) | (right_nibble & 0xf);
                bytes.push_back(b2);
            };

            switch (f) {
                case Format::F1:
                case Format::F3: {
                    if (!holds_alternative<OperandsNone>(node.operand)) {
                        error(_node, "Expected no operands but got " + operand_to_str(node.operand));
                    }
                    bytes.reserve(f == Format::F3 ? 3 : 1);
                    auto b1 = static_cast<Byte_t>(node.mnemonic->opcode);
                    bytes.push_back(b1);
                    if (f == Format::F3) {
                        bytes[0] |= 0b11;
                        bytes.push_back(0);
                        bytes.push_back(0);
                    }
                    break;
                }
                case Format::F2_num: {
                    if (!holds_alternative<OperandsNum>(node.operand)) {
                        error(_node, "Expected no operands but got " + operand_to_str(node.operand));
                    }
                    auto& op = get<OperandsNum>(node.operand);
                    to_bytes_f2(op.num, 0);
                    break;
                }
                case Format::F2_reg:{
                    if (!holds_alternative<OperandsReg>(node.operand)) {
                        error(_node, "Expected no operands but got " + operand_to_str(node.operand));
                    }
                    auto& op = get<OperandsReg>(node.operand);
                    to_bytes_f2(static_cast<Byte_t>(op.reg), 0);
                    break;
                }
                case Format::F2_reg_num: {
                    if (!holds_alternative<OperandsRegNum>(node.operand)) {
                        error(_node, "Expected no operands but got " + operand_to_str(node.operand));
                    }
                    auto& op = get<OperandsRegNum>(node.operand);
                    to_bytes_f2(static_cast<Byte_t>(op.reg), op.num);
                    break;
                }
                case Format::F2_reg_reg: {
                    if (!holds_alternative<OperandsRegReg>(node.operand)) {
                        error(_node, "Expected no operands but got " + operand_to_str(node.operand));
                    }
                    auto& op = get<OperandsRegReg>(node.operand);
                    to_bytes_f2(static_cast<Byte_t>(op.reg_1), static_cast<Byte_t>(op.reg_2));
                    break;
                }
                case Format::F3_4_mem: {
                    if (!holds_alternative<OperandsExpr>(node.operand)) {
                        error(_node, "Expected no operands but got " + operand_to_str(node.operand));
                    }
                    bytes.reserve(node.get_size());

                    auto& op = get<OperandsExpr>(node.operand);


                    if (op.expression->is_imported()) {
                        if (!node.flags.is_extended()) {
                            error(_node, "Can not use imported symbol on non-F4 instruction");
                        }
                    }

                    if (!op.expression->resolved_value.has_value()) {
                        error(op.expression.get(), "Unresolved expression");
                    }

                    auto target_address = op.expression->resolved_value.value();

                    if (node.flags.is_extended()) {
                        if (op.expression->is_imported()) {
                            if (op.expression->get_type() != Ast::NodeType::SymbolExpression) {
                                error(op.expression.get(), "Can not use imported symbols in a complex expression");
                            }

                            auto& expr = *dynamic_cast<Ast::SymbolExpression*>(op.expression.get());

                            add_relocation(expr.symbol_name, node.location + 1, 5);
                        }
                        else if (!op.expression->is_absolute()) {
                            add_relocation(nullopt, node.location + 1, 5);
                        }

                        Byte_t b1 = static_cast<Byte_t>(node.mnemonic->opcode) | node.flags.get_ni();
                        Byte_t b2 = (node.flags.get_xbpe() << 4) | ((target_address >> 16) & 0x0f);
                        Byte_t b3 = (target_address >> 8) & 0xff;
                        Byte_t b4 = target_address & 0xff;
                        bytes.push_back(b1);
                        bytes.push_back(b2);
                        bytes.push_back(b3);
                        bytes.push_back(b4);
                    } else {
                        [&]() {
                            if (op.expression->is_absolute()) {
                                return;
                            }

                            auto pc = static_cast<int>(node.location + node.get_size());
                            auto pc_relative = target_address - pc;

                            if (-2048 <= pc_relative && pc_relative < 2048) {
                                node.flags.set_pc_relative(true);
                                target_address = pc_relative;
                                return;
                            }

                            if (base_register.has_value()) {
                                int base_relative = target_address - base_register.value(); // NOLINT(cppcoreguidelines-narrowing-conversions)

                                if (0 <= base_relative && base_relative < 4096) {
                                    node.flags.set_base_relative(true);
                                    target_address = base_relative;
                                    return;
                                }
                            }

                            if (node.flags.is_immediate() ?
                                (-2048 <= target_address && target_address < 2048) :
                                (0 <= target_address && target_address < 2048)) {
                                add_relocation(nullopt, node.location + 1, 3);
                                return;
                            }

                            if (node.flags.is_simple() && 0 <= target_address && target_address < (1 << 15)) {
                                node.flags.set_ni(0b11);
                                return;
                            }

                            error(_node, "Can't resolve addressing");
                        }();

                        Byte_t b1 = static_cast<Byte_t>(node.mnemonic->opcode) | node.flags.get_ni();
                        bytes.push_back(b1);

                        Byte_t b2 = node.flags.is_sic()
                                ? (node.flags.is_indexed() << 7) | ((target_address >> 8) & 0x7f)
                                : (node.flags.get_xbpe() << 4) | ((target_address >> 8) & 0x0f);
                        bytes.push_back(b2);
                        Byte_t b3 = target_address & 0xff;
                        bytes.push_back(b3);
                    }
                    break;
                }
                default:
                    error(_node, "Unexpected instruction format");
            }
            add_bytes(bytes, node.location);
            break;
        }
        default:
            break;
    }
}

void ProgramObjectGenerator::leave(Ast::Node* _node, Ast::NodeType type) {
    switch (type) {
        case Ast::NodeType::Section: {
            auto& node = to_node(Section);
            flush_pending_bytes();
            flush_m_records();
            os << 'E';
            print_hex(os, node.name.has_value() ? 0 : program->execution_start_address.value_or(0), 6);
            os << endl;
            break;
        }
        default:
            break;
    }
}

ProgramObjectGenerator::ProgramObjectGenerator(ostream& _os)
: os(_os) {}

void ProgramObjectGenerator::add_bytes(vector<Byte_t> const& bytes, Address_t location) {
    if (bytes.empty()) return;

    if (pending_load_address != location) {
        flush_pending_bytes();
        start_of_load = location;
    }

    int add_index = 0;
    while (add_index < bytes.size()) {
        if (pending_bytes.size() == 30) {
            flush_pending_bytes();
            start_of_load = location + add_index;
        }
        pending_bytes.push_back(bytes[add_index++]);
    }
    pending_load_address = location + add_index;
}

void ProgramObjectGenerator::flush_pending_bytes() {
    if (pending_bytes.empty()) return;

    os << 'T';
    print_hex(os, start_of_load, 6);
    print_hex(os, pending_bytes.size(), 2);
    for (auto const& byte : pending_bytes) {
        print_hex(os, byte, 2);
    }
    os << endl;

    pending_bytes.clear();
}

void ProgramObjectGenerator::flush_m_records() {
    for (auto const& record : m_records) {
        os << 'M';
        print_hex(os, record.address, 6);
        print_hex(os, record.length, 2);
        if (record.symbol.has_value()) {
            os << '+';
            print_label(os, record.symbol.value());
        }
        os << endl;
    }
    m_records.clear();
}

void ProgramObjectGenerator::add_relocation(optional<string> symbol, Address_t where, size_t length) {
    m_records.push_back({std::move(symbol), where, length});
}
