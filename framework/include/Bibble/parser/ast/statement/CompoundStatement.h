// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_COMPOUNDSTATEMENT_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_COMPOUNDSTATEMENT_H

#include "Bibble/parser/ast/Node.h"

namespace parser {
    class CompoundStatement : public ASTNode {
    public:
        CompoundStatement(symbol::ScopePtr scope, std::vector<ASTNodePtr> body, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        std::vector<ASTNodePtr> mBody;
        symbol::ScopePtr mOwnScope;
    };

    using CompoundStatementPtr = std::unique_ptr<CompoundStatement>;
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_COMPOUNDSTATEMENT_H
