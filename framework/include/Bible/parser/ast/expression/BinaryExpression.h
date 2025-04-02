// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_AST_EXPRESSION_BINARYEXPRESSION_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_AST_EXPRESSION_BINARYEXPRESSION_H

#include "Bible/parser/ast/Node.h"

#include "Bible/type/IntegerType.h"

#include <JesusASM/tree/instructions/InsnNode.h>

namespace parser {
    class BinaryExpression : public ASTNode {
    public:
        enum class Operator {
            Add,
            Sub,
            Mul,
            Div,
        };

        BinaryExpression(symbol::Scope* scope, ASTNodePtr left, lexer::TokenType operatorToken, ASTNodePtr right, lexer::Token token);
        BinaryExpression(symbol::Scope* scope, ASTNodePtr left, Operator op, ASTNodePtr right, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        ASTNodePtr mLeft;
        ASTNodePtr mRight;
        Operator mOperator;
    };

    using BinaryExpressionPtr = std::unique_ptr<BinaryExpression>;
}

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_AST_EXPRESSION_BINARYEXPRESSION_H
