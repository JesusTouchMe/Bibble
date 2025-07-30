// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_BREAKSTATEMENT_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_BREAKSTATEMENT_H 1

#include "Bibble/parser/ast/Node.h"

namespace parser {
    class BreakStatement : public ASTNode {
    public:
        BreakStatement(symbol::Scope* scope, std::string name, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        std::string mName;
    };

    using BreakStatementPtr = std::unique_ptr<BreakStatement>;
}

#endif // BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_BREAKSTATEMENT_H
