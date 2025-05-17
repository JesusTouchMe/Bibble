// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/statement/ReturnStatement.h"

#include <format>

namespace parser {
    ReturnStatement::ReturnStatement(symbol::Scope* scope, ASTNodePtr returnValue, lexer::Token token)
        : ASTNode(scope, std::move(token))
        , mReturnValue(std::move(returnValue)) {}

    void ReturnStatement::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        if (mReturnValue != nullptr) {
            mReturnValue->codegen(builder, ctx, diag, false);
        }

        builder.createReturn(mType);
    }

    void ReturnStatement::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        if (mReturnValue != nullptr) {
            mReturnValue->semanticCheck(diag, exit, false);
        }

        if (!statement) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("'{}return{}' statement used as an expression",
                                           fmt::bold, fmt::defaults));
            exit = true;
        }
    }

    void ReturnStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        if (mReturnValue != nullptr) {
            mReturnValue->typeCheck(diag, exit);
        }

        Type* returnType = mScope->getCurrentReturnType();
        if (returnType->isVoidType()) {
            if (mReturnValue != nullptr && !mReturnValue->getType()->isVoidType()) {
                diag.compilerError(mErrorToken.getStartLocation(),
                                   mErrorToken.getEndLocation(),
                                   std::format("value of type '{}{}{}' is not compatible with return type of '{}{}{}'",
                                               fmt::bold, mReturnValue->getType()->getName(), fmt::defaults,
                                               fmt::bold, returnType->getName(), fmt::defaults));
                exit = true;
            }
        } else {
            if (mReturnValue == nullptr) {
                diag.compilerError(mErrorToken.getStartLocation(),
                                   mErrorToken.getEndLocation(),
                                   std::format("non-void function returning '{}{}{}' cannot return '{}void{}'",
                                               fmt::bold, returnType->getName(), fmt::defaults,
                                               fmt::bold, fmt::defaults));
                exit = true;
            } else if (returnType != mReturnValue->getType()) {
                if (mReturnValue->implicitCast(diag, returnType)) {
                    mReturnValue = Cast(mReturnValue, returnType);
                } else {
                    diag.compilerError(mErrorToken.getStartLocation(),
                                       mErrorToken.getEndLocation(),
                                       std::format("value of type '{}{}{}' is not compatible with return type of '{}{}{}'",
                                                   fmt::bold, mReturnValue->getType()->getName(), fmt::defaults,
                                                   fmt::bold, returnType->getName(), fmt::defaults));
                    exit = true;
                }
            }
        }

        if (mReturnValue != nullptr) mType = mReturnValue->getType();
        else mType = Type::Get("void");
    }

    bool ReturnStatement::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}