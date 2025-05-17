// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_VARIABLEEXPRESSION_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_VARIABLEEXPRESSION_H

#include "Bibble/parser/ast/Node.h"

namespace parser {
    class VariableExpression : public ASTNode {
    public:
        VariableExpression(symbol::Scope* scope, std::string name, lexer::Token token);
        VariableExpression(symbol::Scope* scope, std::vector<std::string> names, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

        std::string getName();
        std::vector<std::string> getNames();
        bool isQualified();
        bool isImplicitMember();

    private:
        std::vector<std::string> mNames;
        bool mIsImplicitThis;

        std::string reconstructNames();
    };

    using VariableExpressionPtr = std::unique_ptr<VariableExpression>;
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_VARIABLEEXPRESSION_H
