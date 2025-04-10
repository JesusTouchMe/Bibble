// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_IFSTATEMENT_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_IFSTATEMENT_H

#include "Bibble/parser/ast/Node.h"

namespace parser {
    class IfStatement : public ASTNode {
    public:
        IfStatement(symbol::ScopePtr scope, ASTNodePtr condition, ASTNodePtr body, ASTNodePtr elseBody, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        ASTNodePtr mCondition;
        ASTNodePtr mBody;
        ASTNodePtr mElseBody;

        symbol::ScopePtr mOwnScope;
    };

    using IfStatementPtr = std::unique_ptr<IfStatement>;
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_IFSTATEMENT_H
