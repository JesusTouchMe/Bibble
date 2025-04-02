// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/Node.h"

#include <iostream>

namespace parser {
    bool ASTNode::implicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        if (triviallyImplicitCast(diag, destType)) {
            return true;
        }

        auto cast = mType->castTo(destType);
        if (cast == Type::CastLevel::Implicit || cast == Type::CastLevel::ImplicitWarning) {
            if (cast == Type::CastLevel::ImplicitWarning) {
                diag.compilerWarning("implicit", mErrorToken.getStartLocation(), mErrorToken.getEndLocation(), mType->getImplicitCastWarning(destType));
            }
            return true;
        }

        return false;
    }

    ASTNodePtr ASTNode::Cast(ASTNodePtr& node, Type* destType) {
        if (node->mType == destType) {
            return std::move(node);
        }

        std::cout << "TODO: cast expr\n";
        std::exit(1);
    }
}