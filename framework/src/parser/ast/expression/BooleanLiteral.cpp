// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/expression/BooleanLiteral.h"

namespace parser {
    BooleanLiteral::BooleanLiteral(symbol::Scope* scope, bool value, lexer::Token token)
        : ASTNode(scope, Type::Get("bool"), std::move(token))
        , mValue(value) {}

    void BooleanLiteral::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        if (!statement)
            builder.createLdc(mType, mValue);
    }

    void BooleanLiteral::ccodegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, codegen::Label* trueLabel, codegen::Label* falseLabel) {
        if (mValue) builder.createJump(trueLabel);
        else builder.createJump(falseLabel);
    }

    void BooleanLiteral::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {

    }

    void BooleanLiteral::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {

    }

    bool BooleanLiteral::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        if (destType->isIntegerType()) {
            mType = destType;
            return true;
        }

        return false;
    }
}