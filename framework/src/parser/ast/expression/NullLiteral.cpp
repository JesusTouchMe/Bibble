// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/expression/NullLiteral.h"

namespace parser {
    NullLiteral::NullLiteral(symbol::Scope* scope, lexer::Token token)
            : ASTNode(scope, Type::Get("handle"), std::move(token)) {}

    void NullLiteral::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) {
        builder.createLdc(mType, nullptr);
    }

    void NullLiteral::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {

    }

    void NullLiteral::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {

    }

    bool NullLiteral::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        if (destType->isClassType() || destType->isHandleType()) {
            mType = destType;
            return true;
        }

        return false;
    }
}