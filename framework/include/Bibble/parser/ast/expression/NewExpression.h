// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_NEWEXPRESSION_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_NEWEXPRESSION_H 1

#include "Bibble/parser/ast/Node.h"

namespace parser {
    class NewExpression : public ASTNode {
    public:
        NewExpression(symbol::Scope* scope, Type* type, std::vector<ASTNodePtr> parameters, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        std::vector<ASTNodePtr> mParameters;
        symbol::FunctionSymbol* mBestViableConstructor;

        bool mIsStatement;

        symbol::FunctionSymbol* getBestViableConstructor(diagnostic::Diagnostics& diag, bool& exit);
    };

    using NewExpressionPtr = std::unique_ptr<NewExpression>;
}

#endif // BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_NEWEXPRESSION_H
