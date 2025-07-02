// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/expression/MemberAccess.h"
#include "Bibble/parser/ast/expression/UnaryExpression.h"
#include "Bibble/parser/ast/expression/VariableExpression.h"

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

            case lexer::TokenType::DoublePlus:
                mOperator = Operator::Increment;
                break;

            case lexer::TokenType::DoubleMinus:
                mOperator = Operator::Decrement;
                break;

            case lexer::TokenType::Bang:
                mOperator = Operator::LogicalNot;
                break;

            default:
                break;
        }
    }

    UnaryExpression::UnaryExpression(symbol::Scope* scope, ASTNodePtr operand, Operator op, bool postfix, lexer::Token token)
        : ASTNode(scope, std::move(token))
        , mOperand(std::move(operand))
        , mOperator(op)
        , mPostfix(postfix) {}

    void UnaryExpression::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        if (mOperator != Operator::Increment && mOperator != Operator::Decrement) {
            mOperand->codegen(builder, ctx, diag, statement);
            if (statement) return;
        }


        auto createIncrement = [&](i16 increment) {
            if (auto var = dynamic_cast<VariableExpression*>(mOperand.get())) {
                if (var->isImplicitMember()) {
                    auto scopeOwner = var->getScope()->findOwner();

                    auto* local = var->getScope()->findLocal("this");
                    if (local == nullptr) {
                        diag.fatalError("scope is owned by a class, but no 'this' local exists");
                    }

                    auto* field = scopeOwner->getField(var->getName());

                    builder.createLoad(local->type, local->index);
                    builder.createDup(local->type);
                    builder.createGetField(scopeOwner->getType(), field->type, field->name);

                    if (mPostfix) {
                        if (!statement) builder.createDupX1(field->type);
                        builder.createLdc(field->type, (i64) 1);
                        builder.createAdd(field->type);
                    } else {
                        builder.createLdc(field->type, (i64) 1);
                        builder.createAdd(field->type);
                        if (!statement) builder.createDupX1(field->type);
                    }

                    builder.createSetField(scopeOwner->getType(), field->type, field->name);
                } else {
                    auto* local = var->getScope()->findLocal(var->getName());
                    if (mPostfix) {
                        builder.createLoad(local->type, local->index);
                        builder.createInc(local->type, local->index, increment);
                    } else {
                        builder.createInc(local->type, local->index, increment);
                        builder.createLoad(local->type, local->index);
                    }
                }
            } else if (auto member = dynamic_cast<MemberAccess*>(mOperand.get())) {
                member->getClass()->codegen(builder, ctx, diag, true);
                builder.createDup(member->getClass()->getType());

                auto* field = member->getClassSymbol()->getField(member->getId());

                builder.createGetField(member->getClassType(), field->type, field->name);

                if (mPostfix) {
                    if (!statement) builder.createDupX1(field->type);
                    builder.createLdc(field->type, (i64) 1);
                    builder.createAdd(field->type);
                } else {
                    builder.createLdc(field->type, (i64) 1);
                    builder.createAdd(field->type);
                    if (!statement) builder.createDupX1(field->type);
                }

                builder.createSetField(member->getClassType(), field->type, field->name);
            }
        };

        switch (mOperator) {
            case Operator::Negate:
                builder.createNeg(mOperand->getType());
                break;
            case Operator::Increment:
                createIncrement(1);
                break;
            case Operator::Decrement:
                createIncrement(-1);
                break;
        }
    }

    void UnaryExpression::ccodegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, codegen::Label* trueLabel, codegen::Label* falseLabel) {
        if (mOperator == Operator::LogicalNot) {
            mOperand->ccodegen(builder, ctx, diag, falseLabel, trueLabel);
        }
    }

    void UnaryExpression::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        mOperand->semanticCheck(diag, exit, false);
    }

    void UnaryExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mOperand->typeCheck(diag, exit);

        switch (mOperator) {
            case Operator::Negate:
            case Operator::Increment:
            case Operator::Decrement:
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

            case Operator::LogicalNot: {
                Type* boolType = Type::Get("bool");
                if (!mOperand->getType()->isBooleanType()) {
                    if (mOperand->implicitCast(diag, boolType)) {
                        mOperand = Cast(mOperand, boolType);
                    } else {
                        diag.compilerError(mErrorToken.getStartLocation(),
                                           mErrorToken.getEndLocation(),
                                           std::format("no match for '{}operator {}{}' with type '{}{}{}'",
                                                       fmt::bold, mErrorToken.getName(), fmt::defaults,
                                                       fmt::bold, mOperand->getType()->getName(), fmt::defaults));
                        exit = true;
                    }
                }
                mType = boolType;
                break;
            }
        }
    }

    bool UnaryExpression::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}
