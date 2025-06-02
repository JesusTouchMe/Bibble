// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/expression/MemberAccess.h"

#include <format>

namespace parser {
    MemberAccess::MemberAccess(symbol::Scope* scope, ASTNodePtr classNode, std::string id, lexer::Token operatorToken, lexer::Token token)
        : ASTNode(scope, std::move(token))
        , mOperatorToken(std::move(operatorToken))
        , mClass(std::move(classNode))
        , mId(std::move(id)) {}

    ASTNode* MemberAccess::getClass() const {
        return mClass.get();
    }

    std::string_view MemberAccess::getId() const {
        return mId;
    }

    symbol::ClassSymbol* MemberAccess::getClassSymbol() const {
        return mClassSymbol;
    }

    ClassType* MemberAccess::getClassType() const {
        return mClassType;
    }

    void MemberAccess::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        if (statement) return;

        mClass->codegen(builder, ctx, diag, false);

        if (mClass->getType()->isArrayType()) {
            builder.createArrayLength(mClass->getType());
        } else {
            auto field = mClassSymbol->getField(mId);

            builder.createGetField(mClassType, field->type, field->name);
        }
    }

    void MemberAccess::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        mClass->semanticCheck(diag, exit, statement);

        if (mClass->getType()->isArrayType()) {
            return;
        }

        auto field = mClassSymbol->getField(mId);
        auto scopeOwner = mScope->findOwner();
        ClassType* classType = nullptr;

        if (scopeOwner != nullptr) {
            classType = scopeOwner->getType();
        }

        if (field != nullptr && (field->modifiers & MODULEWEB_FIELD_MODIFIER_PRIVATE) && mClassType != classType) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("'{}{}{}' is a private member of class '{}{}{}'",
                                           fmt::bold, mId, fmt::defaults,
                                           fmt::bold, mClassType->getName(), fmt::defaults));
            exit = true;
        }

        auto method = mClassSymbol->getMethod(mId);
        if (method != nullptr && (method->modifiers & MODULEWEB_FUNCTION_MODIFIER_PRIVATE) && mClassType != classType) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("'{}{}{}' is a private member of class '{}{}{}'",
                                           fmt::bold, mId, fmt::defaults,
                                           fmt::bold, mClassType->getName(), fmt::defaults));
            exit = true;
        }

        if (statement) {
            diag.compilerWarning("unused-value",
                                 mErrorToken.getStartLocation(),
                                 mErrorToken.getEndLocation(),
                                 "expression result unused");
        }
    }

    void MemberAccess::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mClass->typeCheck(diag, exit);

        if (mClass->getType()->isArrayType() && mId == "length") {
            mType = Type::Get("int");
            return;
        }

        if (!mClass->getType()->isClassType()) {
            diag.compilerError(mOperatorToken.getStartLocation(),
                               mOperatorToken.getEndLocation(),
                               std::format("'{}operator .{}' used on non-class value '{}{}{}'",
                                           fmt::bold, fmt::defaults,
                                           fmt::bold, mClass->getErrorToken().getText(), fmt::defaults));
            exit = true;
            return;
        }

        mClassType = static_cast<ClassType*>(mClass->getType());
        mClassSymbol = mScope->findClass({ std::string(mClassType->getModuleName()), std::string(mClassType->getName()) });

        if (mClassSymbol == nullptr) {
            diag.fatalError("class symbol not found (should never happen)");
        }

        auto field = mClassSymbol->getField(mId);

        if (field != nullptr) {
            mType = field->type;
        } else {
            auto method = mClassSymbol->getMethod(mId);

            if (method != nullptr) {
                mType = method->type->getReturnType();
            } else {
                diag.compilerError(mErrorToken.getStartLocation(),
                                   mErrorToken.getEndLocation(),
                                   std::format("class '{}{}{}' has no member named '{}{}{}'",
                                               fmt::bold, mClassType->getName(), fmt::defaults,
                                               fmt::bold, mId, fmt::defaults));
                exit = true;
            }
        }
    }

    bool MemberAccess::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}