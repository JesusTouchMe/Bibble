// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/expression/BinaryExpression.h"
#include "Bibble/parser/ast/expression/MemberAccess.h"
#include "Bibble/parser/ast/expression/VariableExpression.h"

#include "Bibble/type/ArrayType.h"
#include "Bibble/type/ViewType.h"

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

            case lexer::TokenType::DoubleAmpersand:
                mOperator = Operator::LogicalAnd;
                break;

            case lexer::TokenType::DoublePipe:
                mOperator = Operator::LogicalOr;
                break;

            case lexer::TokenType::Equal:
                mOperator = Operator::Assign;
                break;

            case lexer::TokenType::PlusEqual:
                mOperator = Operator::AddAssign;
                break;

            case lexer::TokenType::MinusEqual:
                mOperator = Operator::SubAssign;
                break;

            case lexer::TokenType::LeftBracket:
                mOperator = Operator::Index;
                break;

            default:
                break;
        }
    }

    BinaryExpression::BinaryExpression(symbol::Scope* scope, ASTNodePtr left, BinaryExpression::Operator op, ASTNodePtr right, lexer::Token token)
            : ASTNode(scope, std::move(token))
            , mLeft(std::move(left))
            , mOperator(op)
            , mRight(std::move(right)) {}

    void BinaryExpression::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        if (mOperator != Operator::Assign && mOperator != Operator::AddAssign && mOperator != Operator::SubAssign && mOperator != Operator::Index) {
            mLeft->codegen(builder, ctx, diag, statement);
            mRight->codegen(builder, ctx, diag, statement);
        }

        if (statement && mOperator != Operator::Assign && mOperator != Operator::AddAssign && mOperator != Operator::SubAssign) return;

        auto createAssign = [&]() {
            if (auto variableExpression = dynamic_cast<VariableExpression*>(mLeft.get())) {
                if (variableExpression->isImplicitMember()) {
                    auto scopeOwner = mScope->findOwner();

                    symbol::LocalSymbol* local = mScope->findLocal("this");
                    if (local == nullptr) {
                        diag.fatalError("scope is owned by a class, but no 'this' local exists");
                    }

                    auto field = scopeOwner->getField(variableExpression->getName());

                    builder.createLoad(local->type, local->index);

                    if (mOperator == Operator::AddAssign || mOperator == Operator::SubAssign) {
                        builder.createDup(local->type);
                        builder.createGetField(scopeOwner->getType(), field->type, field->name);
                        mRight->codegen(builder, ctx, diag, false);

                        if (mOperator == Operator::AddAssign) builder.createAdd(mLeft->getType());
                        else builder.createSub(mLeft->getType());

                        if (!statement) builder.createDupX1(mRight->getType());

                        builder.createSetField(scopeOwner->getType(), field->type, field->name);
                    } else {
                        mRight->codegen(builder, ctx, diag, false);

                        if (!statement) builder.createDupX1(mRight->getType());

                        builder.createSetField(scopeOwner->getType(), field->type, field->name);
                    }
                } else {
                    symbol::LocalSymbol* local = mScope->findLocal(variableExpression->getName());
                    if (local == nullptr) {
                        diag.compilerError(mErrorToken.getStartLocation(),
                                           mErrorToken.getEndLocation(),
                                           std::format("couldn't find local variable '{}{}{}'",
                                                       fmt::bold, variableExpression->getName(), fmt::defaults));
                        std::exit(1);
                    }

                    if (mOperator == Operator::AddAssign || mOperator == Operator::SubAssign) {
                        builder.createLoad(local->type, local->index);
                        mRight->codegen(builder, ctx, diag, false);
                        builder.createAdd(mLeft->getType());
                        if (!statement) builder.createDup(mRight->getType());
                        builder.createStore(local->type, local->index);
                    } else {
                        mRight->codegen(builder, ctx, diag, false);
                        if (!statement) builder.createDup(mRight->getType());

                        builder.createStore(local->type, local->index);
                    }
                }
            } else if (auto memberAccess = dynamic_cast<MemberAccess*>(mLeft.get())) {
                memberAccess->getClass()->codegen(builder, ctx, diag, false);

                auto field = memberAccess->getClassSymbol()->getField(memberAccess->getId());

                if (mOperator == Operator::AddAssign || mOperator == Operator::SubAssign) {
                    builder.createDup(memberAccess->getClassType());
                    builder.createGetField(memberAccess->getClassType(), field->type, field->name);
                    mRight->codegen(builder, ctx, diag, false);

                    if (mOperator == Operator::AddAssign) builder.createAdd(mLeft->getType());
                    else builder.createSub(mLeft->getType());

                    if (!statement) builder.createDupX1(mRight->getType());

                    builder.createSetField(memberAccess->getClassType(), field->type, field->name);
                } else {
                    mRight->codegen(builder, ctx, diag, false);
                    builder.createSetField(memberAccess->getClassType(), field->type, field->name);

                    if (!statement) builder.createDupX1(mRight->getType());
                }
            } else if (auto binaryExpr = dynamic_cast<BinaryExpression*>(mLeft.get()); binaryExpr->mOperator == Operator::Index) {
                ASTNode* array = binaryExpr->mLeft.get();
                ASTNode* index = binaryExpr->mRight.get();

                array->codegen(builder, ctx, diag, false);
                index->codegen(builder, ctx, diag, false);

                if (mOperator == Operator::AddAssign || mOperator == Operator::SubAssign) {
                    builder.createDup2(array->getType(), index->getType());
                    builder.createArrayLoad(array->getType());

                    mRight->codegen(builder, ctx, diag, false);

                    if (mOperator == Operator::AddAssign) builder.createAdd(mLeft->getType());
                    else builder.createSub(mLeft->getType());

                    if (!statement) builder.createDupX2(mRight->getType());

                    builder.createArrayStore(array->getType());
                } else {
                    mRight->codegen(builder, ctx, diag, false);
                    if (!statement) builder.createDupX2(mRight->getType());

                    builder.createArrayStore(array->getType());
                }
            } else {
                diag.fatalError("TODO: error message");
            }
        };

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

            case Operator::Equal: {
                auto trueLabel = builder.createLabel("");
                auto falseLabel = builder.createLabel("");
                auto mergeLabel = builder.createLabel("");

                builder.createJumpCmpNE(mLeft->getType(), falseLabel.get());
                builder.insertLabel(std::move(trueLabel)); // this is required to be here because the code verifier of jesusasm is shit. fix it
                builder.createLdc(mType, true);
                builder.createJump(mergeLabel.get());
                builder.insertLabel(std::move(falseLabel));
                builder.createLdc(mType, false);
                builder.insertLabel(std::move(mergeLabel));

                break;
            }
            case Operator::NotEqual: {
                auto trueLabel = builder.createLabel("");
                auto falseLabel = builder.createLabel("");
                auto mergeLabel = builder.createLabel("");

                builder.createJumpCmpEQ(mLeft->getType(), falseLabel.get());
                builder.insertLabel(std::move(trueLabel)); // this is required to be here because the code verifier of jesusasm is shit. fix it
                builder.createLdc(mType, true);
                builder.createJump(mergeLabel.get());
                builder.insertLabel(std::move(falseLabel));
                builder.createLdc(mType, false);
                builder.insertLabel(std::move(mergeLabel));

                break;
            }
            case Operator::LessThan: {
                auto trueLabel = builder.createLabel("");
                auto falseLabel = builder.createLabel("");
                auto mergeLabel = builder.createLabel("");

                builder.createJumpCmpGE(mLeft->getType(), falseLabel.get());
                builder.insertLabel(std::move(trueLabel)); // this is required to be here because the code verifier of jesusasm is shit. fix it
                builder.createLdc(mType, true);
                builder.createJump(mergeLabel.get());
                builder.insertLabel(std::move(falseLabel));
                builder.createLdc(mType, false);
                builder.insertLabel(std::move(mergeLabel));

                break;
            }
            case Operator::GreaterThan: {
                auto trueLabel = builder.createLabel("");
                auto falseLabel = builder.createLabel("");
                auto mergeLabel = builder.createLabel("");

                builder.createJumpCmpLE(mLeft->getType(), falseLabel.get());
                builder.insertLabel(std::move(trueLabel)); // this is required to be here because the code verifier of jesusasm is shit. fix it
                builder.createLdc(mType, true);
                builder.createJump(mergeLabel.get());
                builder.insertLabel(std::move(falseLabel));
                builder.createLdc(mType, false);
                builder.insertLabel(std::move(mergeLabel));

                break;
            }
            case Operator::LessEqual: {
                auto trueLabel = builder.createLabel("");
                auto falseLabel = builder.createLabel("");
                auto mergeLabel = builder.createLabel("");

                builder.createJumpCmpGT(mLeft->getType(), falseLabel.get());
                builder.insertLabel(std::move(trueLabel)); // this is required to be here because the code verifier of jesusasm is shit. fix it
                builder.createLdc(mType, true);
                builder.createJump(mergeLabel.get());
                builder.insertLabel(std::move(falseLabel));
                builder.createLdc(mType, false);
                builder.insertLabel(std::move(mergeLabel));

                break;
            }
            case Operator::GreaterEqual: {
                auto trueLabel = builder.createLabel("");
                auto falseLabel = builder.createLabel("");
                auto mergeLabel = builder.createLabel("");

                builder.createJumpCmpLT(mLeft->getType(), falseLabel.get());
                builder.insertLabel(std::move(trueLabel)); // this is required to be here because the code verifier of jesusasm is shit. fix it
                builder.createLdc(mType, true);
                builder.createJump(mergeLabel.get());
                builder.insertLabel(std::move(falseLabel));
                builder.createLdc(mType, false);
                builder.insertLabel(std::move(mergeLabel));

                break;
            }

            case Operator::Assign:
            case Operator::AddAssign:
            case Operator::SubAssign:
                createAssign();
                break;

            case Operator::Index:
                mLeft->codegen(builder, ctx, diag, false);
                mRight->codegen(builder, ctx, diag, false);

                builder.createArrayLoad(mLeft->getType());

                break;
        }
    }

    void BinaryExpression::ccodegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, codegen::Label* trueLabel, codegen::Label* falseLabel) {
        if (mOperator == Operator::LogicalAnd) {
            auto newLabel = builder.createLabel("");

            mLeft->ccodegen(builder, ctx, diag, newLabel.get(), falseLabel);
            builder.insertLabel(std::move(newLabel));
            mRight->ccodegen(builder, ctx, diag, trueLabel, falseLabel);
        } else if (mOperator == Operator::LogicalOr) {
            auto newLabel = builder.createLabel("");

            mLeft->ccodegen(builder, ctx, diag, trueLabel, newLabel.get());
            builder.insertLabel(std::move(newLabel));
            mRight->ccodegen(builder, ctx, diag, trueLabel, falseLabel);
        } else {
            mLeft->codegen(builder, ctx, diag, false);
            mRight->codegen(builder, ctx, diag, false);

            switch (mOperator) {
                case Operator::Equal:
                    builder.createJumpCmpNE(mLeft->getType(), falseLabel);
                    break;
                case Operator::NotEqual:
                    builder.createJumpCmpEQ(mLeft->getType(), falseLabel);
                    break;
                case Operator::LessThan:
                    builder.createJumpCmpGE(mLeft->getType(), falseLabel);
                    break;
                case Operator::GreaterThan:
                    builder.createJumpCmpLE(mLeft->getType(), falseLabel);
                    break;
                case Operator::LessEqual:
                    builder.createJumpCmpGT(mLeft->getType(), falseLabel);
                    break;
                case Operator::GreaterEqual:
                    builder.createJumpCmpLT(mLeft->getType(), falseLabel);
                    break;
                default:
                    return;
            }

            builder.createJump(trueLabel);
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
                    if (mRight->implicitCast(diag, mLeft->getType())) {
                        mRight = Cast(mRight, mLeft->getType());
                        mType = mLeft->getType();
                    } else if (mLeft->implicitCast(diag, mRight->getType())) {
                        mLeft = Cast(mLeft, mRight->getType());
                        mType = mRight->getType();
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

            case Operator::LogicalAnd:
            case Operator::LogicalOr: {
                Type* boolType = Type::Get("bool");

                auto checkOperand = [this, &diag, &exit, boolType](ASTNodePtr& operand) {
                    if (!operand->getType()->isBooleanType()) {
                        if (operand->implicitCast(diag, boolType)) {
                            operand = Cast(operand, boolType);
                        } else {
                            diag.compilerError(mErrorToken.getStartLocation(),
                                        mErrorToken.getEndLocation(),
                                        std::format("no match for '{}operator {}{}' with the given types '{}{}{}' and '{}{}{}'",
                                                    fmt::bold, mErrorToken.getName(), fmt::defaults,
                                                    fmt::bold, mLeft->getType()->getName(), fmt::defaults,
                                                    fmt::bold, mRight->getType()->getName(), fmt::defaults));
                            exit = true;
                        }
                    }
                };

                checkOperand(mLeft);
                checkOperand(mRight);
                mType = boolType;
                break;
            }

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

                if (auto binaryExpr = dynamic_cast<BinaryExpression*>(mLeft.get()); binaryExpr != nullptr && binaryExpr->mOperator == Operator::Index) {
                    if (binaryExpr->mLeft->getType()->isViewType()) {
                        diag.compilerError(mErrorToken.getStartLocation(),
                                            mErrorToken.getEndLocation(),
                                            std::format("attempt to mutate array view of type '{}{}{}'",
                                                        fmt::bold, static_cast<ViewType*>(binaryExpr->mLeft->getType())->getBaseType()->getName(), fmt::defaults));
                        exit = true;
                        return;
                    }
                }

                mType = mLeft->getType();

                break;

            case Operator::AddAssign:
            case Operator::SubAssign:
                if (mLeft->getType()->isIntegerType() || mLeft->getType()->isCharType()) {
                    if (mLeft->getType() != mRight->getType()) {
                        if (mRight->implicitCast(diag, mLeft->getType())) {
                            mRight = Cast(mRight, mLeft->getType());
                        } else {
                            diag.compilerError(mErrorToken.getStartLocation(),
                                       mErrorToken.getEndLocation(),
                                       std::format("no match for '{}operator {}{}' with the given types '{}{}{}' and '{}{}{}'",
                                                   fmt::bold, mErrorToken.getName(), fmt::defaults,
                                                   fmt::bold, mLeft->getType()->getName(), fmt::defaults,
                                                   fmt::bold, mRight->getType()->getName(), fmt::defaults));
                            exit = true;
                        }
                        mType = mLeft->getType();
                    }
                } else {
                    diag.compilerError(mErrorToken.getStartLocation(),
                                       mErrorToken.getEndLocation(),
                                       std::format("no match for '{}operator {}{}' with the given types '{}{}{}' and '{}{}{}'",
                                                   fmt::bold, mErrorToken.getName(), fmt::defaults,
                                                   fmt::bold, mLeft->getType()->getName(), fmt::defaults,
                                                   fmt::bold, mRight->getType()->getName(), fmt::defaults));
                    exit = true;
                }
                break;

            case Operator::Index:
                if (!mLeft->getType()->isArrayType() && !mLeft->getType()->isArrayView()) {
                    diag.compilerError(mErrorToken.getStartLocation(),
                                       mErrorToken.getEndLocation(),
                                       std::format("no match for '{}operator []{}' with type '{}{}{}'",
                                                   fmt::bold, fmt::defaults,
                                                   fmt::bold, mLeft->getType()->getName(), fmt::defaults));
                    exit = true;
                }

                if (!mRight->getType()->isIntegerType()) {
                    if (mRight->implicitCast(diag, Type::Get("int"))) {
                        mRight = Cast(mRight, Type::Get("int"));
                    } else {
                        diag.compilerError(mErrorToken.getStartLocation(),
                                           mErrorToken.getEndLocation(),
                                           std::format("no match for '{}operator []{}' with index type '{}{}{}'",
                                                       fmt::bold, fmt::defaults,
                                                       fmt::bold, mRight->getType()->getName(), fmt::defaults));
                        exit = true;
                    }
                }

                if (mLeft->getType()->isArrayType()) {
                    mType = static_cast<ArrayType*>(mLeft->getType())->getElementType();
                } else {
                    mType = static_cast<ArrayType*>(static_cast<ViewType*>(mLeft->getType())->getBaseType())->getElementType();
                }

                break;
        }
    }

    bool BinaryExpression::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}
