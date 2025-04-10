// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_BINARYEXPRESSION_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_BINARYEXPRESSION_H

#include "Bibble/parser/ast/Node.h"

#include "Bibble/type/IntegerType.h"

#include <JesusASM/tree/instructions/InsnNode.h>

namespace parser {
    class BinaryExpression : public ASTNode {
    public:
        enum class Operator {
            Add,
            Sub,
            Mul,
            Div,

            Equal,
            NotEqual,
            LessThan,
            GreaterThan,
            LessEqual,
            GreaterEqual,
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

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_BINARYEXPRESSION_H
