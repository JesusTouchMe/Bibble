// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/expression/BinaryExpression.h"
#include "Bibble/parser/ast/expression/MemberAccess.h"
#include "Bibble/parser/ast/expression/VariableExpression.h"

#include <format>

namespace parser {
    BinaryExpression::BinaryExpression(symbol::Scope* scope, ASTNodePtr left, lexer::TokenType operatorToken,ASTNodePtr right, lexer::Token token)
            : ASTNode(scope, std::move(token))
            , mLeft(std::move(left))
            , mRight(std::move(right)) {
        switch (operatorToken) {
            case lexer::TokenType::Plus:
                mOperator = Operator::Add;
                break;

            case lexer::TokenType::Minus:
                mOperator = Operator::Sub;
                break;

            case lexer::TokenType::Star:
                mOperator = Operator::Mul;
                break;

            case lexer::TokenType::Slash:
                mOperator = Operator::Div;
                break;

            case lexer::TokenType::DoubleEqual:
                mOperator = Operator::Equal;
                break;

            case lexer::TokenType::BangEqual:
                mOperator = Operator::NotEqual;
                break;

            case lexer::TokenType::LessThan:
                mOperator = Operator::LessThan;
                break;

            case lexer::TokenType::GreaterThan:
                mOperator = Operator::GreaterThan;
                break;

            case lexer::TokenType::LessEqual:
                mOperator = Operator::LessEqual;
                break;

            case lexer::TokenType::GreaterEqual:
                mOperator = Operator::GreaterEqual;
                break;

            case lexer::TokenType::Equal:
                mOperator = Operator::Assign;

            default:
                break;
        }
    }

    BinaryExpression::BinaryExpression(symbol::Scope* scope, ASTNodePtr left, BinaryExpression::Operator op, ASTNodePtr right, lexer::Token token)
            : ASTNode(scope, std::move(token))
            , mLeft(std::move(left))
            , mOperator(op)
            , mRight(std::move(right)) {}

    void BinaryExpression::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) {
        if (mOperator != Operator::Assign) {
            mLeft->codegen(builder, ctx, diag);
            mRight->codegen(builder, ctx, diag);
        }

        switch (mOperator) {
            case Operator::Add:
                if (!mType->isIntegerType() && !mType->isCharType()) {
                    diag.compilerError(mErrorToken.getStartLocation(),
                                       mErrorToken.getEndLocation(),
                                       "can't add a non-integral type");
                    std::exit(EXIT_FAILURE);
                }

                builder.createAdd(mType);
                break;
            case Operator::Sub:
                if (!mType->isIntegerType() && !mType->isCharType()) {
                    diag.compilerError(mErrorToken.getStartLocation(),
                                       mErrorToken.getEndLocation(),
                                       "can't subtract a non-integral type");
                    std::exit(EXIT_FAILURE);
                }

                builder.createSub(mType);
                break;

            case Operator::Mul:
                if (!mType->isIntegerType()) {
                    diag.compilerError(mErrorToken.getStartLocation(),
                                       mErrorToken.getEndLocation(),
                                       "can't multiply a non-integer type");
                    std::exit(EXIT_FAILURE);
                }

                builder.createMul(mType);
                break;

            case Operator::Div:
                if (!mType->isIntegerType()) {
                    diag.compilerError(mErrorToken.getStartLocation(),
                                       mErrorToken.getEndLocation(),
                                       "can't divide a non-integer type");
                    std::exit(EXIT_FAILURE);
                }

                builder.createDiv(mType);
                break;

            case Operator::Equal:
                builder.createCmpEQ(mLeft->getType());
                break;
            case Operator::NotEqual:
                builder.createCmpNE(mLeft->getType());
                break;
            case Operator::LessThan:
                builder.createCmpLT(mLeft->getType());
                break;
            case Operator::GreaterThan:
                builder.createCmpGT(mLeft->getType());
                break;
            case Operator::LessEqual:
                builder.createCmpLE(mLeft->getType());
                break;
            case Operator::GreaterEqual:
                builder.createCmpGE(mLeft->getType());
                break;

            case Operator::Assign:
                if (auto variableExpression = dynamic_cast<VariableExpression*>(mLeft.get())) {
                    if (variableExpression->isImplicitMember()) {
                        auto scopeOwner = mScope->findOwner();

                        symbol::LocalSymbol* local = mScope->findLocal("this");
                        if (local == nullptr) {
                            diag.fatalError("scope is owned by a class, but no 'this' local exists");
                        }

                        builder.createLoad(local->type, local->index);

                        mRight->codegen(builder, ctx, diag);

                        auto field = scopeOwner->getField(variableExpression->getName());
                        builder.createSetField(scopeOwner->getType(), field->type, field->name);
                    } else {
                        symbol::LocalSymbol* local = mScope->findLocal(variableExpression->getName());
                        if (local == nullptr) {
                            diag.compilerError(mErrorToken.getStartLocation(),
                                               mErrorToken.getEndLocation(),
                                               std::format("couldn't find local variable '{}{}{}'",
                                                           fmt::bold, variableExpression->getName(), fmt::defaults));
                            std::exit(1);
                        }

                        mRight->codegen(builder, ctx, diag);

                        builder.createStore(local->type, local->index);
                    }
                } else if (auto memberAccess = dynamic_cast<MemberAccess*>(mLeft.get())) {
                    memberAccess->getClass()->codegen(builder, ctx, diag);

                    mRight->codegen(builder, ctx, diag);

                    auto field = memberAccess->getClassSymbol()->getField(memberAccess->getId());
                    builder.createSetField(memberAccess->getClassType(), field->type, field->name);
                } else {
                    diag.fatalError("TODO: error message");
                }

                break;
        }
    }

    void BinaryExpression::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        mLeft->semanticCheck(diag, exit, false);
        mRight->semanticCheck(diag, exit, false);
    }

    void BinaryExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mLeft->typeCheck(diag, exit);
        mRight->typeCheck(diag, exit);

        switch (mOperator) {
            case Operator::Add:
            case Operator::Sub:
            case Operator::Mul:
            case Operator::Div:
                if (mLeft->getType() != mRight->getType() && mLeft->getType()->isIntegerType() && mRight->getType()->isIntegerType()) {
                    auto leftType = static_cast<IntegerType*>(mLeft->getType());
                    auto rightType = static_cast<IntegerType*>(mRight->getType());

                    if (leftType->getSize() > rightType->getSize()) {
                        if (mRight->implicitCast(diag, leftType)) {
                            mRight = Cast(mRight, leftType);
                        }

                        mType = leftType;
                    } else {
                        if (mLeft->implicitCast(diag, rightType)) {
                            mLeft = Cast(mLeft, rightType);
                        }

                        mType = rightType;
                    }
                }

                if (mLeft->getType() != mRight->getType() || !mLeft->getType()->isIntegerType()) {
                    diag.compilerError(mErrorToken.getStartLocation(),
                                       mErrorToken.getEndLocation(),
                                       std::format("no match for '{}operator {}{}' with the given types '{}{}{}' and '{}{}{}'",
                                                   fmt::bold, mErrorToken.getName(), fmt::defaults,
                                                   fmt::bold, mLeft->getType()->getName(), fmt::defaults,
                                                   fmt::bold, mRight->getType()->getName(), fmt::defaults));
                    exit = true;
                }

                mType = mLeft->getType();
                break;


            case Operator::Equal:
            case Operator::NotEqual:
            case Operator::LessThan:
            case Operator::GreaterThan:
            case Operator::LessEqual:
            case Operator::GreaterEqual:
                if (mLeft->getType() != mRight->getType()) {
                    if (mLeft->getType()->getStackSlots() > mRight->getType()->getStackSlots()) {
                        if (mRight->implicitCast(diag, mLeft->getType())) {
                            mRight = Cast(mRight, mLeft->getType());
                        }

                        mType = mLeft->getType();
                    } else if (mRight->getType()->getStackSlots() > mLeft->getType()->getStackSlots()) {
                        if (mLeft->implicitCast(diag, mRight->getType())) {
                            mLeft = Cast(mLeft, mRight->getType());
                        }

                        mType = mRight->getType();
                    } else {
                        if (mRight->implicitCast(diag, mLeft->getType())) {
                            mRight = Cast(mRight, mLeft->getType());
                            mType = mLeft->getType();
                        } else if (mLeft->implicitCast(diag, mRight->getType())) {
                            mLeft = Cast(mLeft, mRight->getType());
                            mType = mRight->getType();
                        }
                    }
                }

                if (mLeft->getType() != mRight->getType()) {
                    diag.compilerError(mErrorToken.getStartLocation(),
                                       mErrorToken.getEndLocation(),
                                       std::format("no match for '{}operator {}{}' with the given types '{}{}{}' and '{}{}{}'",
                                                   fmt::bold, mErrorToken.getName(), fmt::defaults,
                                                   fmt::bold, mLeft->getType()->getName(), fmt::defaults,
                                                   fmt::bold, mRight->getType()->getName(), fmt::defaults));
                    exit = true;
                }

                mType = Type::Get("bool");
                break;

            case Operator::Assign:
                if (mLeft->getType() != mRight->getType()) {
                    if (mRight->implicitCast(diag, mLeft->getType())) {
                        mRight = Cast(mRight, mLeft->getType());
                    } else {
                        diag.compilerError(mErrorToken.getStartLocation(),
                                           mErrorToken.getEndLocation(),
                                           std::format("no match for '{}operator {}{}' with the given types '{}{}{}' and '{}{}{}''",
                                                       fmt::bold, mErrorToken.getName(), fmt::defaults,
                                                       fmt::bold, mLeft->getType()->getName(), fmt::defaults,
                                                       fmt::bold, mRight->getType()->getName(), fmt::defaults));
                        exit = true;
                    }
                }

                break;
        }
    }

    bool BinaryExpression::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}