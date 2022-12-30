//
// Created by Lenart on 27/12/2022.
//

#ifndef ASS2_SICCST_H
#define ASS2_SICCST_H

#include <cstddef>
#include <memory>
#include <set>
#include <iostream>
#include "ast/Visitor.h"
#include "../common/SicTypes.h"
#include "ast/SymbolTable.h"
#include "ast/SicAST.h"

using std::shared_ptr;
using std::set;
using std::ostream;

class ProgramAssembler {
public:
    explicit ProgramAssembler(shared_ptr<Ast::Program> program);
    void set_ast_stream(ostream* s) { ast_stream = s; }
    void set_lst_stream(ostream* s) { lst_stream = s; }
    void set_obj_stream(ostream* s) { obj_stream = s; }
    void assemble_program();
private:
    shared_ptr<Ast::Program> program;
    ostream* ast_stream = &std::cout;
    ostream* lst_stream = &std::cout;
    ostream* obj_stream = &std::cout;
};

class AstTreeDump : public Ast::Visitor {
public:
    explicit AstTreeDump(ostream& _os);

    void visit(Ast::Node* _node, Ast::NodeType type) override;
    void leave(Ast::Node* _node, Ast::NodeType type) override;
private:
    ostream& os;
    size_t m_indent {};
};

class LstGenerator : public Ast::Visitor {
public:
    explicit LstGenerator(ostream& _os);
public:
    void visit(Ast::Node* _node, Ast::NodeType type) override;
    void leave(Ast::Node* _node, Ast::NodeType type) override;

private:
    ostream& os;
};

class AbsoluteExpressionResolver : public Ast::Visitor {
public:
    void visit(Ast::Node* _node, Ast::NodeType type) override;

    void leave(Ast::Node* _node, Ast::NodeType type) override;
};

class DefineSymbolAndLocationVisitor : public Ast::Visitor {
public:
    void visit(Ast::Node* _node, Ast::NodeType type) override;
    void leave(Ast::Node* _node, Ast::NodeType type) override;

private:
    void try_define_command(Ast::Command* command);
    void try_define_symbol(Ast::Node* _node, SymbolTable::Symbol const& symbol);

    Address_t locctr;
    Ast::Program* program;
    Ast::Section* section;
    set<string> symbols_to_export;
};

class SymbolExpressionResolver : public Ast::Visitor {
public:
    void visit(Ast::Node* _node, Ast::NodeType type) override;
    void leave(Ast::Node* _node, Ast::NodeType type) override;
private:
    Ast::Program* program {};
    Ast::Section* section {};
};

class ProgramObjectGenerator : public Ast::Visitor {
public:
    explicit ProgramObjectGenerator(ostream& _os);
    void visit(Ast::Node* _node, Ast::NodeType type) override;
    void leave(Ast::Node* _node, Ast::NodeType type) override;
private:
    void add_relocation(optional<string> symbol, Address_t where, size_t length);
    void add_bytes(vector<Byte_t> const& bytes, Address_t location);
    void flush_pending_bytes();
    void flush_m_records();

    struct MRecord {
        optional<string> symbol;
        Address_t address {};
        size_t length {};
    };

    vector<MRecord> m_records {};
    vector<Byte_t> pending_bytes {};
    Address_t start_of_load {};
    Address_t pending_load_address {};
    optional<Address_t> base_register {};
    Ast::Program* program {};
    ostream& os;
};
#endif //ASS2_SICCST_H
