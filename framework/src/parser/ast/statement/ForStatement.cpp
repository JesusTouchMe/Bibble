// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/statement/ForStatement.h"

#include <format>

namespace parser {
    ForStatement::ForStatement(symbol::ScopePtr scope, ASTNodePtr init, ASTNodePtr condition, ASTNodePtr it, ASTNodePtr body, std::string name, lexer::Token token)
        : ASTNode(scope->parent, std::move(token))
        , mInit(std::move(init))
        , mCondition(std::move(condition))
        , mIt(std::move(it))
        , mBody(std::move(body))
        , mName(std::move(name))
        , mOwnScope(std::move(scope)) {}

    void ForStatement::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        codegen::LabelPtr bodyLabel = builder.createLabel("");
        codegen::LabelPtr itLabel = builder.createLabel("");
        codegen::LabelPtr mergeLabel = builder.createLabel("");

        codegen::Label* bodyLabelPtr = bodyLabel.get();

        mScope->loopContext = symbol::LoopContext(mergeLabel.get(), itLabel.get(), mName);

        mInit->codegen(builder, ctx, diag, true);
        mCondition->ccodegen(builder, ctx, diag, bodyLabel.get(), mergeLabel.get());

        builder.insertLabel(std::move(bodyLabel));
        mBody->codegen(builder, ctx, diag, true);
        builder.createJump(itLabel.get());

        builder.insertLabel(std::move(itLabel));
        mIt->codegen(builder, ctx, diag, true);
        mCondition->ccodegen(builder, ctx, diag, bodyLabelPtr, mergeLabel.get());

        builder.insertLabel(std::move(mergeLabel));
    }

    void ForStatement::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        mInit->semanticCheck(diag, exit, true);
        mCondition->semanticCheck(diag, exit, false);
        mIt->semanticCheck(diag, exit, true);
        mBody->semanticCheck(diag, exit, true);

        if (!statement) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("'{}while{}' loop used as an expression",
                                           fmt::bold, fmt::defaults));
            exit = true;
        }
    }

    void ForStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mInit->typeCheck(diag, exit);
        mCondition->typeCheck(diag, exit);
        mIt->typeCheck(diag, exit);
        mBody->typeCheck(diag, exit);

        if (!mCondition->getType()->isBooleanType()) {
            auto boolType = Type::Get("bool");

            if (mCondition->implicitCast(diag, boolType)) {
                mCondition = Cast(mCondition, boolType);
            } else {
                diag.compilerError(mCondition->getErrorToken().getStartLocation(),
                                   mCondition->getErrorToken().getEndLocation(),
                                   std::format("value of type '{}{}{}' cannot be used as a condition in for loop",
                                               fmt::bold, mCondition->getType()->getName(), fmt::defaults));
            }
        }
    }

    bool ForStatement::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}
