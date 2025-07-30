// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_FORSTATEMENT_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_FORSTATEMENT_H 1

#include "Bibble/parser/ast/Node.h"

namespace parser {
    class ForStatement : public ASTNode {
    public:
        ForStatement(symbol::ScopePtr scope, ASTNodePtr init, ASTNodePtr condition, ASTNodePtr it, ASTNodePtr body, std::string name, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        ASTNodePtr mInit;
        ASTNodePtr mCondition;
        ASTNodePtr mIt;
        ASTNodePtr mBody;
        std::string mName;

        symbol::ScopePtr mOwnScope;
    };

    using ForStatementPtr = std::unique_ptr<ForStatement>;
}

#endif // BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_FORSTATEMENT_H
