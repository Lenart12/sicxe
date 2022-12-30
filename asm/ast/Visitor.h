//
// Created by Lenart on 08/12/2022.
//

#ifndef ASS2_VISITOR_H
#define ASS2_VISITOR_H

#include "Forward.h"

namespace Ast {

class Visitor {
public:
    virtual void visit(Node* _node, NodeType type) = 0;
    virtual void leave(Node* _node, NodeType type) = 0;
};

}



#endif //ASS2_VISITOR_H
