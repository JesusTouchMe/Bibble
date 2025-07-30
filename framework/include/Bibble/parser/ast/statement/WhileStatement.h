// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_WHILESTATEMENT_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_WHILESTATEMENT_H 1

#include "Bibble/parser/ast/Node.h"

namespace parser {
    class WhileStatement : public ASTNode {
    public:
        WhileStatement(symbol::ScopePtr scope, ASTNodePtr condition, ASTNodePtr body, std::string name, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        ASTNodePtr mCondition;
        ASTNodePtr mBody;
        std::string mName;

        symbol::ScopePtr mOwnScope;
    };

    using WhileStatementPtr = std::unique_ptr<WhileStatement>;
}

#endif // BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_STATEMENT_WHILESTATEMENT_H
