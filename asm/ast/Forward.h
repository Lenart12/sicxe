//
// Created by Lenart on 08/12/2022.
//

#ifndef ASS2_FORWARD_H
#define ASS2_FORWARD_H

namespace Ast {

class Visitor;

#define ENUMERATE_NODES()             \
    ENUMERATE_NODE(Node)              \
    ENUMERATE_NODE(Block)             \
    ENUMERATE_NODE(Program)           \
    ENUMERATE_NODE(Section)           \
    ENUMERATE_NODE(Command)           \
    ENUMERATE_NODE(Instruction)       \
    ENUMERATE_NODE(Directive)         \
    ENUMERATE_NODE(Expression)        \
    ENUMERATE_NODE(UnaryExpression)  \
    ENUMERATE_NODE(BinaryExpression)  \
    ENUMERATE_NODE(SymbolExpression)  \
    ENUMERATE_NODE(NumericExpression)

// Forward declare nodes
#define ENUMERATE_NODE(NodeName) \
    class NodeName;
ENUMERATE_NODES()
#undef ENUMERATE_NODE

// Create node type
#define ENUMERATE_NODE(NodeName) \
    NodeName,
enum class NodeType {
    ENUMERATE_NODES()
};
#undef ENUMERATE_NODE


}
#endif //ASS2_FORWARD_H
