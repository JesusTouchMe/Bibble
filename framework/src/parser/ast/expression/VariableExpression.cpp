// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/expression/VariableExpression.h"

#include "Bibble/type/ViewType.h"

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

    void VariableExpression::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        if (statement) return;

        if (mIsImplicitThis) {
            auto scopeOwner = mScope->findOwner();

            symbol::LocalSymbol* local = mScope->findLocal("this");
            if (local == nullptr) {
                diag.fatalError("scope is owned by a class, but no 'this' local exists");
            }

            builder.createLoad(local->type, local->index);

            auto field = scopeOwner->getField(mNames.back());
            builder.createGetField(scopeOwner->getType(), field->type, field->name);

            return;
        }

        symbol::GlobalVarSymbol* global = isQualified() ? mScope->findGlobalVar(mNames) : mScope->findGlobalVar(mNames.back());
        if (global != nullptr) {
            builder.createGetGlobal(global->type, global->moduleName, global->name);
            return;
        }

        symbol::LocalSymbol* local = mScope->findLocal(mNames.back());
        if (local == nullptr) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("couldn't find local variable '{}{}{}'",
                                           fmt::bold, mNames.back(), fmt::defaults));
            std::exit(1);
        }

        builder.createLoad(local->type, local->index);
    }

    void VariableExpression::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {

    }

    void VariableExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        symbol::ClassSymbol* scopeOwner = mScope->findOwner();

        if (scopeOwner != nullptr && !isQualified() && mScope->findLocal(mNames.back()) == nullptr) {
            symbol::LocalSymbol* localThis = mScope->findLocal("this");
            if (localThis == nullptr) {
                diag.fatalError("scope is owned by a class, but no 'this' local exists");
            }

            bool view = localThis->type->isViewType();

            auto field = scopeOwner->getField(mNames.back());
            if (field != nullptr) {
                if (field->type->isReferenceType() && view) {
                    mType = ViewType::Create(field->type);
                } else {
                    mType = field->type;
                }

                mIsImplicitThis = true;
                return;
            } else {
                if (!view) {
                    auto method = scopeOwner->getMethod(mNames.back());
                    if (method != nullptr) {
                        mType = method->type->getReturnType();
                        mIsImplicitThis = true;
                        return;
                    }
                }

                auto method = scopeOwner->getMethod(mNames.back() + ".v");
                if (method != nullptr) {
                    mType = method->type->getReturnType();
                    mIsImplicitThis = true;
                    return;
                }
            }
        }

        symbol::FunctionSymbol* func = isQualified() ? mScope->findFunction(mNames, nullptr) : mScope->findFunction(mNames.back(), nullptr);
        if (func != nullptr) {
            mType = func->type;
            return;
        }

        symbol::GlobalVarSymbol* global = isQualified() ? mScope->findGlobalVar(mNames) : mScope->findGlobalVar(mNames.back());
        if (global != nullptr) {
            mType = global->type;
            return;
        }

        if (isQualified()) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("undeclared identifier: '{}{}{}'",
                                           fmt::bold, reconstructNames(), fmt::defaults));
            exit = true;
            return;
        }

        symbol::LocalSymbol* local = mScope->findLocal(mNames.back());
        if (local != nullptr) {
            mType = local->type;
            return;
        }

        diag.compilerError(mErrorToken.getStartLocation(),
                           mErrorToken.getEndLocation(),
                           std::format("undeclared identifier: '{}{}{}'",
                                       fmt::bold, reconstructNames(), fmt::defaults));
        exit = true;
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
