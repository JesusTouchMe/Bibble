// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/statement/CompoundStatement.h"

#include <format>

namespace parser {
    CompoundStatement::CompoundStatement(symbol::ScopePtr scope, std::vector<ASTNodePtr> body, lexer::Token token)
        : ASTNode(scope->parent, Type::Get("void"), std::move(token))
        , mBody(std::move(body))
        , mOwnScope(std::move(scope)) {}

    void CompoundStatement::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) {
        for (auto& node : mBody) {
            node->codegen(builder, ctx, diag);
        }
    }

    void CompoundStatement::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        for (auto& node : mBody) {
            node->semanticCheck(diag, exit, true);
        }
    }

    void CompoundStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        for (auto& node : mBody) {
            node->typeCheck(diag, exit);
        }
    }

    bool CompoundStatement::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}