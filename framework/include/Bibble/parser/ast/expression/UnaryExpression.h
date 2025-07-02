// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_UNARYEXPRESSION_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_UNARYEXPRESSION_H

#include "Bibble/parser/ast/Node.h"

namespace parser {
    class UnaryExpression : public ASTNode {
    public:
        enum class Operator {
            Negate,

            Increment,
            Decrement,

            LogicalNot
        };

        UnaryExpression(symbol::Scope* scope, ASTNodePtr operand, lexer::TokenType operatorToken, bool postfix, lexer::Token token);
        UnaryExpression(symbol::Scope* scope, ASTNodePtr operand, Operator op, bool postfix, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) override;
        void ccodegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, codegen::Label* trueLabel, codegen::Label* falseLabel) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        ASTNodePtr mOperand;
        Operator mOperator;
        bool mPostfix;
    };

    using UnaryExpressionPtr = std::unique_ptr<UnaryExpression>;
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_UNARYEXPRESSION_H
