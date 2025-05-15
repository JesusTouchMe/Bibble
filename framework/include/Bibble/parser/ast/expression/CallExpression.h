// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_CALLEXPRESSION_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_CALLEXPRESSION_H

#include "Bibble/parser/ast/Node.h"

namespace parser {
    class CallExpression : public ASTNode {
    public:
        CallExpression(symbol::Scope* scope, ASTNodePtr callee, std::vector<ASTNodePtr> parameters);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        ASTNodePtr mCallee;
        std::vector<ASTNodePtr> mParameters;
        symbol::FunctionSymbol* mBestViableFunction;

        bool mIsMemberFunction;
        bool mIsStatement;

        symbol::FunctionSymbol* getBestViableFunction(diagnostic::Diagnostics& diag, bool& exit);
    };

    using CallExpressionPtr = std::unique_ptr<CallExpression>;
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_CALLEXPRESSION_H
