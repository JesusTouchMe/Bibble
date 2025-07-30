// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_CONTINUESTATEMENT_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_CONTINUESTATEMENT_H 1

#include "Bibble/parser/ast/Node.h"

namespace parser {
    class ContinueStatement : public ASTNode {
    public:
        ContinueStatement(symbol::Scope* scope, std::string name, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        std::string mName;
    };

    using ContinueStatementPtr = std::unique_ptr<ContinueStatement>;
}

#endif // BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_CONTINUESTATEMENT_H
