// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/expression/UnaryExpression.h"

#include <format>

namespace parser {
    UnaryExpression::UnaryExpression(symbol::Scope* scope, ASTNodePtr operand, lexer::TokenType operatorToken, bool postfix, lexer::Token token)
        : ASTNode(scope, std::move(token))
        , mOperand(std::move(operand))
        , mPostfix(postfix) {
        switch (operatorToken) {
            case lexer::TokenType::Minus:
                mOperator = Operator::Negate;
                break;

            default:
                break;
        }
    }

    UnaryExpression::UnaryExpression(symbol::Scope* scope, ASTNodePtr operand, UnaryExpression::Operator op, bool postfix, lexer::Token token)
        : ASTNode(scope, std::move(token))
        , mOperand(std::move(operand))
        , mOperator(op)
        , mPostfix(postfix) {}

    void UnaryExpression::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        mOperand->codegen(builder, ctx, diag, statement);

        if (statement) return;

        switch (mOperator) {
            case Operator::Negate:
                builder.createNeg(mOperand->getType());
                break;
        }
    }

    void UnaryExpression::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        mOperand->semanticCheck(diag, exit, false);
    }

    void UnaryExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mOperand->typeCheck(diag, exit);

        switch (mOperator) {
            case Operator::Negate:
                if (!mOperand->getType()->isIntegerType() && !mOperand->getType()->isCharType()) {
                    diag.compilerError(mErrorToken.getStartLocation(),
                                       mErrorToken.getEndLocation(),
                                       std::format("no match for '{}operator {}{}' with type '{}{}{}'",
                                                   fmt::bold, mErrorToken.getName(), fmt::defaults,
                                                   fmt::bold, mOperand->getType()->getName(), fmt::defaults));
                    exit = true;
                } else {
                    mType = mOperand->getType();
                }
                break;
        }
    }

    bool UnaryExpression::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}