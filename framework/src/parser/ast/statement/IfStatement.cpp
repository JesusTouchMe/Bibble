// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/statement/IfStatement.h"

#include <format>

namespace parser {

    IfStatement::IfStatement(symbol::ScopePtr scope, ASTNodePtr condition, ASTNodePtr body, ASTNodePtr elseBody, lexer::Token token)
        : ASTNode(scope->parent, Type::Get("void"), std::move(token))
        , mCondition(std::move(condition))
        , mBody(std::move(body))
        , mElseBody(std::move(elseBody))
        , mOwnScope(std::move(scope)) {}

    void IfStatement::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) {
        mCondition->codegen(builder, ctx, diag);

        codegen::LabelPtr trueLabel = builder.createLabel("");
        codegen::LabelPtr falseLabel;
        codegen::LabelPtr mergeLabel = builder.createLabel("");

        if (mElseBody) {
            falseLabel = builder.createLabel("");

            builder.createCondJump(trueLabel.get(), falseLabel.get());
        } else {
            builder.createCondJump(trueLabel.get(), mergeLabel.get());
        }

        builder.insertLabel(std::move(trueLabel));
        mBody->codegen(builder, ctx, diag);
        builder.createJump(mergeLabel.get());

        if (mElseBody) {
            builder.insertLabel(std::move(falseLabel));
            mElseBody->codegen(builder, ctx, diag);
            builder.createJump(mergeLabel.get());
        }

        builder.insertLabel(std::move(mergeLabel));
    }

    void IfStatement::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        mCondition->semanticCheck(diag, exit, true);
        mBody->semanticCheck(diag, exit, false);
        if (mElseBody) mElseBody->semanticCheck(diag, exit, false);

        if (!statement) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("'{}if{}' statement used as an expression",
                                           fmt::bold, fmt::defaults));
            exit = true;
        }
    }

    void IfStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mCondition->typeCheck(diag, exit);
        mBody->typeCheck(diag, exit);
        if (mElseBody) mElseBody->typeCheck(diag, exit);

        if (!mCondition->getType()->isBooleanType()) {
            auto boolType = Type::Get("bool");

            if (mCondition->implicitCast(diag, boolType)) {
                mCondition = Cast(mCondition, boolType);
            } else {
                diag.compilerError(mCondition->getErrorToken().getStartLocation(),
                                   mCondition->getErrorToken().getEndLocation(),
                                   std::format("value of type '{}{}{}' cannot be used as a condition in if statement",
                                               fmt::bold, mCondition->getType()->getName(), fmt::defaults));
            }
        }
    }

    bool IfStatement::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}