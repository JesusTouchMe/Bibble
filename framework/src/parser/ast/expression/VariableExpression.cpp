// Copyright 2025 JesusTouchMe

#include "Bible/parser/ast/expression/VariableExpression.h"

#include <format>

namespace parser {
    VariableExpression::VariableExpression(symbol::Scope* scope, std::string name, lexer::Token token)
        : ASTNode(scope, std::move(token))
        , mNames({std::move(name)})
        , mIsImplicitThis(false) {}

    VariableExpression::VariableExpression(symbol::Scope* scope, std::vector<std::string> names, lexer::Token token)
        : ASTNode(scope, std::move(token))
        , mNames(std::move(names))
        , mIsImplicitThis(false) {}

    void VariableExpression::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) {
        if (mIsImplicitThis) {
            diag.fatalError("Unimplemented");
        }

        if (isQualified()) diag.fatalError("Unimplemented 2");

        symbol::LocalSymbol* local = mScope->findLocal(mNames.back());
        if (local == nullptr) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("couldn't find local variable '{}{}{}' (this message should never be seen but you never know)",
                                           fmt::bold, mNames.back(), fmt::defaults));
            std::exit(1);
        }

        builder.createLoad(local->type, local->index);
    }

    void VariableExpression::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {

    }

    void VariableExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        symbol::ClassSymbol* scopeOwner = mScope->findOwner();

        if (scopeOwner != nullptr) {
            auto field = scopeOwner->getField(mNames.back());
            if (field != nullptr) {
                mType = field->type;
                mIsImplicitThis = true;
                return;
            } else {
                auto method = scopeOwner->getMethod(mNames.back());
                if (method != nullptr) {
                    mType = method->type->getReturnType();
                    mIsImplicitThis = true;
                    return;
                }
            }
        }

        // if (isQualified()) globalScope stuff

        symbol::LocalSymbol* local = mScope->findLocal(mNames.back());
        if (local == nullptr) {
            symbol::FunctionSymbol* func = mScope->findFunction(mNames.back(), nullptr);
            if (func == nullptr) {
                diag.compilerError(mErrorToken.getStartLocation(),
                                   mErrorToken.getEndLocation(),
                                   std::format("undeclared identifier '{}{}{}'",
                                               fmt::bold, reconstructNames(), fmt::defaults));
                exit = true;
            } else {
                mType = func->type;
            }
        } else {
            mType = local->type;
        }
    }

    bool VariableExpression::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }

    std::string VariableExpression::getName() {
        return mNames.back();
    }

    std::vector<std::string> VariableExpression::getNames() {
        return mNames;
    }

    bool VariableExpression::isQualified() {
        return mNames.size() > 1;
    }

    bool VariableExpression::isImplicitMember() {
        return mIsImplicitThis;
    }

    std::string VariableExpression::reconstructNames() {
        std::string ret;
        for (auto it = mNames.begin(); it != mNames.end() - 1; ++it) {
            ret += (*it);
            ret += "::";
        }
        ret += mNames.back();

        return ret;
    }
}
