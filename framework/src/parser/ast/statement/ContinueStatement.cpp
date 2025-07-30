// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/statement/ContinueStatement.h"

#include <format>

namespace parser {
    ContinueStatement::ContinueStatement(symbol::Scope* scope, std::string name, lexer::Token token)
        : ASTNode(scope, std::move(token))
        , mName(std::move(name)) {}

    void ContinueStatement::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        auto label = mScope->getContinueLabel(mName);
        if (label == nullptr) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               "continue statement not within a loop");
            std::exit(1);
        }

        builder.createJump(label);
    }

    void ContinueStatement::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        if (!statement) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("'{}continue{}' statement used as an expression",
                                           fmt::bold, fmt::defaults));
            exit = true;
        }
    }

    void ContinueStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
    }

    bool ContinueStatement::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}
