// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/expression/StringLiteral.h"

namespace parser {
    StringLiteral::StringLiteral(symbol::Scope* scope, std::string value, lexer::Token token)
        : ASTNode(scope, Type::Get("string"), std::move(token))
        , mValue(std::move(value)) {}

    void StringLiteral::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) {
        builder.createLdc(mValue);
    }

    void StringLiteral::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {

    }

    void StringLiteral::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {

    }

    bool StringLiteral::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}