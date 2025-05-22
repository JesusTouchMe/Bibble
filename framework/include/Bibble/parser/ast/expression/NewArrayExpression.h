// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_NEWARRAYEXPRESSION_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_NEWARRAYEXPRESSION_H 1

#include "Bibble/parser/ast/Node.h"

#include "Bibble/type/ArrayType.h"

namespace parser {
    class NewArrayExpression : public ASTNode {
    public:
        NewArrayExpression(symbol::Scope* scope, ArrayType* arrayType, ASTNodePtr length, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        ASTNodePtr mLength;
    };

    using NewArrayExpressionPtr = std::unique_ptr<NewArrayExpression>;
}

#endif // BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_NEWARRAYEXPRESSION_H
