// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/expression/MemberAccess.h"

#include "Bibble/type/ViewType.h"

#include <format>

namespace parser {
    MemberAccess::MemberAccess(symbol::Scope* scope, ASTNodePtr classNode, std::string id, lexer::Token operatorToken, lexer::Token token)
        : ASTNode(scope, std::move(token))
        , mOperatorToken(std::move(operatorToken))
        , mClass(std::move(classNode))
        , mId(std::move(id))
        , mClassSymbol(nullptr)
        , mClassType(nullptr) {}

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

        if (mClass->getType()->isArrayView()) {
            builder.createArrayLength(mClass->getType());
        } else {
            auto field = mClassSymbol->getField(mId);

            builder.createGetField(mClassType, field->type, field->name);
        }
    }

    void MemberAccess::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        mClass->semanticCheck(diag, exit, statement);

        if (mClass->getType()->isArrayView()) {
            return;
        }

        auto field = mClassSymbol->getField(mId);
        auto method = mClassSymbol->getMethod(mId);
        auto viewMethod = mClassSymbol->getMethod(mId + ".v");
        auto scopeOwner = mScope->findOwner();
        ClassType* classType = nullptr;

        if (scopeOwner != nullptr) {
            classType = scopeOwner->getType();
        }

        if (field != nullptr && (field->modifiers & MODULEWEB_FIELD_MODIFIER_PRIVATE || field->modifiers & MODULEWEB_FIELD_MODIFIER_PROTECTED) && mClassType != classType && method == nullptr && viewMethod == nullptr) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("'{}{}{}' is a private member of class '{}{}{}'",
                                           fmt::bold, mId, fmt::defaults,
                                           fmt::bold, mClassType->getName(), fmt::defaults));
            exit = true;
        }

        if (method != nullptr && (method->modifiers & MODULEWEB_METHOD_MODIFIER_PRIVATE || method->modifiers & MODULEWEB_METHOD_MODIFIER_PRIVATE) && mClassType != classType) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("'{}{}{}' is a private member of class '{}{}{}'",
                                           fmt::bold, mId, fmt::defaults,
                                           fmt::bold, mClassType->getName(), fmt::defaults));
            exit = true;
        }

        if (viewMethod != nullptr && (viewMethod->modifiers & MODULEWEB_METHOD_MODIFIER_PRIVATE) && mClassType != classType) {
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

        if (mClass->getType()->isArrayView() && mId == "length") {
            mType = Type::Get("long");
            return;
        }

        if (!mClass->getType()->isClassView()) {
            diag.compilerError(mOperatorToken.getStartLocation(),
                               mOperatorToken.getEndLocation(),
                               std::format("'{}operator .{}' used on non-class value '{}{}{}'",
                                           fmt::bold, fmt::defaults,
                                           fmt::bold, mClass->getErrorToken().getText(), fmt::defaults));
            exit = true;
            return;
        }

        if (mClass->getType()->isViewType()) {
            mClassType = static_cast<ClassType*>(static_cast<ViewType*>(mClass->getType())->getBaseType());
        } else {
            mClassType = static_cast<ClassType*>(mClass->getType());
        }
        mClassSymbol = mScope->findClass({ std::string(mClassType->getModuleName()), std::string(mClassType->getName()) });

        if (mClassSymbol == nullptr) {
            diag.fatalError("class symbol not found (should never happen)");
        }

        auto field = mClassSymbol->getField(mId);

        if (field != nullptr) {
            if (mClass->getType()->isViewType()) {
                mType = ViewType::Create(field->type);
            } else {
                mType = field->type;
            }
        } else {
            bool view = mClass->getType()->isViewType();

            if (!view) {
                auto method = mClassSymbol->getMethod(mId);

                if (method != nullptr) {
                    mType = method->type->getReturnType();
                    return;
                }
            }

            auto method = mClassSymbol->getMethod( mId + ".v");

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
