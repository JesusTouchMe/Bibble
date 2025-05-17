// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/expression/CastExpression.h"

#include <format>

namespace parser {
    CastExpression::CastExpression(symbol::Scope* scope, ASTNodePtr value, Type* destType)
        : ASTNode(scope, destType, lexer::Token("", lexer::TokenType::Error, {}, {}))
        , mValue(std::move(value)) {}

    void CastExpression::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        mValue->codegen(builder, ctx, diag, statement);
        if (!statement) builder.createCast(mValue->getType(), mType);
    }

    void CastExpression::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        mValue->semanticCheck(diag, exit, statement);

        if (mType->isVoidType() && !statement) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("cannot cast expression of type '{}{}{}' to '{}void{}'",
                                           fmt::bold, mValue->getType()->getName(), fmt::defaults,
                                           fmt::bold, fmt::defaults));
            exit = true;
        }
    }

    void CastExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mValue->typeCheck(diag, exit);
    }

    bool CastExpression::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return mValue->triviallyImplicitCast(diag, destType);
    }
}