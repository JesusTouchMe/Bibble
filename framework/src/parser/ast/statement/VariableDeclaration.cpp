// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/statement/VariableDeclaration.h"

#include <format>

namespace parser {
    VariableDeclaration::VariableDeclaration(symbol::Scope* scope, Type* type, std::string name, ASTNodePtr initialValue, lexer::Token token)
        : ASTNode(scope, type, std::move(token))
        , mName(std::move(name))
        , mInitialValue(std::move(initialValue)) {}

    void VariableDeclaration::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        if (mInitialValue) {
            auto* local = mScope->findLocal(mName);

            mInitialValue->codegen(builder, ctx, diag, false);
            builder.createStore(local->type, local->index);
        }
    }

    void VariableDeclaration::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        if (mInitialValue) mInitialValue->semanticCheck(diag, exit, false);

        if (!statement) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               "variable declaration used as an expression");
            exit = true;
        }
    }

    void VariableDeclaration::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        if (mType == nullptr) {
            if (!mInitialValue) {
                diag.compilerError(mErrorToken.getStartLocation(),
                                   mErrorToken.getEndLocation(),
                                   std::format("variable '{}{}{}' has an unknown type",
                                               fmt::bold, mName, fmt::defaults));
                exit = true;
                return;
            }

            mInitialValue->typeCheck(diag, exit);

            mType = mInitialValue->getType();
        }

        int* index = mScope->findVariableIndex();
        mScope->locals[mName] = symbol::LocalSymbol(*index, mType);

        *index += mType->getStackSlots();

        if (mType->isVoidType()) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("cannot create variable of type '{}{}{}'",
                                           fmt::bold, mType->getName(), fmt::defaults));
            exit = true;
            return;
        }

        if (mInitialValue) {
            mInitialValue->typeCheck(diag, exit);

            if (mInitialValue->getType() != mType) {
                if (mInitialValue->implicitCast(diag, mType)) {
                    mInitialValue = Cast(mInitialValue, mType);
                } else {
                    diag.compilerError(mErrorToken.getStartLocation(),
                                       mErrorToken.getEndLocation(),
                                       std::format("cannot assign value of type '{}{}{}' to variable with of type '{}{}{}'",
                                                   fmt::bold, mInitialValue->getType()->getName(), fmt::defaults,
                                                   fmt::bold, mType->getName(), fmt::defaults));
                    exit = true;
                }
            }
        }
    }

    bool VariableDeclaration::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}