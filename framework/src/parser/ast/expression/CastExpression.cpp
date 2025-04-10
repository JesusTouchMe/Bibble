// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/expression/CastExpression.h"

namespace parser {
    CastExpression::CastExpression(symbol::Scope* scope, ASTNodePtr value, Type* destType)
        : ASTNode(scope, destType, lexer::Token("", lexer::TokenType::Error, {}, {}))
        , mValue(std::move(value)) {}

    void CastExpression::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) {
        mValue->codegen(builder, ctx, diag);
        builder.createCast(mValue->getType(), mType);
    }

    void CastExpression::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        mValue->semanticCheck(diag, exit, false);
    }

    void CastExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mValue->typeCheck(diag, exit);
    }

    bool CastExpression::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return mValue->triviallyImplicitCast(diag, destType);
    }
}