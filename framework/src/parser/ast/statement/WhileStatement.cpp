// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/statement/WhileStatement.h"

#include <format>

namespace parser {
    WhileStatement::WhileStatement(symbol::ScopePtr scope, ASTNodePtr condition, ASTNodePtr body, std::string name, lexer::Token token)
        : ASTNode(scope->parent, Type::Get("void"), std::move(token))
        , mCondition(std::move(condition))
        , mBody(std::move(body))
        , mName(std::move(name))
        , mOwnScope(std::move(scope)) {}

    void WhileStatement::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        codegen::LabelPtr conditionLabel = builder.createLabel("");
        codegen::LabelPtr bodyLabel = builder.createLabel("");
        codegen::LabelPtr mergeLabel = builder.createLabel("");

        codegen::Label* bodyLabelPtr = bodyLabel.get();

        mOwnScope->loopContext = symbol::LoopContext(mergeLabel.get(), conditionLabel.get(), mName);

        builder.insertLabel(std::move(conditionLabel));
        mCondition->ccodegen(builder, ctx, diag, bodyLabel.get(), mergeLabel.get());

        builder.insertLabel(std::move(bodyLabel));
        mBody->codegen(builder, ctx, diag, true);

        mCondition->ccodegen(builder, ctx, diag, bodyLabelPtr, mergeLabel.get());

        builder.insertLabel(std::move(mergeLabel));
    }

    void WhileStatement::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        mCondition->semanticCheck(diag, exit, false);
        mBody->semanticCheck(diag, exit, true);

        if (!statement) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("'{}while{}' loop used as an expression",
                                           fmt::bold, fmt::defaults));
            exit = true;
        }
    }

    void WhileStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mCondition->typeCheck(diag, exit);
        mBody->typeCheck(diag, exit);

        if (!mCondition->getType()->isBooleanType()) {
            auto boolType = Type::Get("bool");

            if (mCondition->implicitCast(diag, boolType)) {
                mCondition = Cast(mCondition, boolType);
            } else {
                diag.compilerError(mCondition->getErrorToken().getStartLocation(),
                                   mCondition->getErrorToken().getEndLocation(),
                                   std::format("value of type '{}{}{}' cannot be used as a condition in while loop",
                                               fmt::bold, mCondition->getType()->getName(), fmt::defaults));
            }
        }
    }

    bool WhileStatement::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}
