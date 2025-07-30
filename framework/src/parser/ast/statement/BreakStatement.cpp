// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/statement/BreakStatement.h"

#include <format>

namespace parser {
    BreakStatement::BreakStatement(symbol::Scope* scope, std::string name, lexer::Token token)
        : ASTNode(scope, std::move(token))
        , mName(std::move(name)) {}

    void BreakStatement::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        auto label = mScope->getBreakLabel(mName);
        if (label == nullptr) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               "break statement not within a loop");
            std::exit(1);
        }

        builder.createJump(label);
    }

    void BreakStatement::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        if (!statement) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("'{}break{}' statement used as an expression",
                                           fmt::bold, fmt::defaults));
            exit = true;
        }
    }

    void BreakStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
    }

    bool BreakStatement::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}
