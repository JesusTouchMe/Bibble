// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/global/GlobalVar.h"

#include <algorithm>
#include <format>

namespace parser {
    GlobalVar::GlobalVar(std::vector<GlobalVarModifier> modifiers, symbol::Scope* scope, Type* type, std::string name, ASTNodePtr initialValue, lexer::Token token)
        : ASTNode(scope, type, std::move(token))
        , mModifiers(std::move(modifiers))
        , mName(std::move(name))
        , mInitialValue(std::move(initialValue)) {
        u16 realModifiers = 0;
        for (auto modifier : mModifiers) {
            realModifiers |= static_cast<u16>(modifier);
        }

        mScope->createGlobalVar(mName, mType, realModifiers);
    }

    void GlobalVar::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        u16 modifiers = 0;
        for (auto modifier : mModifiers) {
            modifiers |= static_cast<u16>(modifier);
        }

        builder.addGlobalVar(modifiers, mName, mType->getJesusASMType());

        if (mInitialValue != nullptr) {
            symbol::GlobalVarSymbol& symbol = mScope->globalVars.at(mName);

            auto functionType = FunctionType::Create(Type::Get("void"), {});
            auto function = mScope->findFunction("#LinkInit", functionType);
            JesusASM::tree::FunctionNode* functionNode;

            if (function == nullptr) {
                function = mScope->createFunction("#LinkInit", functionType, 0);
                functionNode = builder.addFunction(0, "#LinkInit", functionType->getJesusASMType());
            } else {
                auto it = std::find_if(ctx.getModule()->functions.begin(), ctx.getModule()->functions.end(), [](const auto& node) {
                    return node->name == "#LinkInit";
                });
                if (it == ctx.getModule()->functions.end()) {
                    diag.fatalError("'#LinkInit' symbol declared, but no definition was found");
                }

                functionNode = it->get();
            }

            builder.setInsertPoint(&functionNode->instructions);

            mInitialValue->codegen(builder, ctx, diag, false);
            builder.createSetGlobal(mType, symbol.moduleName, symbol.name);
        }
    }

    void GlobalVar::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        if (mInitialValue != nullptr) mInitialValue->semanticCheck(diag, exit, false);

        if (!statement) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               "global variable declaration used as an expression");
            exit = true;
        }
    }

    void GlobalVar::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        if (mType == nullptr) {
            if (mInitialValue == nullptr) {
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

        if (mType->isVoidType()) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("cannot create variable of type '{}{}{}'",
                                           fmt::bold, mType->getName(), fmt::defaults));
            exit = true;
            return;
        }

        if (mInitialValue != nullptr) {
            mInitialValue->typeCheck(diag, exit);

            if (mInitialValue->getType() != mType) {
                if (mInitialValue->implicitCast(diag, mType)) {
                    mInitialValue = Cast(mInitialValue, mType);
                } else {
                    diag.compilerError(mErrorToken.getStartLocation(),
                                       mErrorToken.getEndLocation(),
                                       std::format("cannot assign value of type '{}{}{}' to variable of type '{}{}{}'",
                                                   fmt::bold, mInitialValue->getType()->getName(), fmt::defaults,
                                                   fmt::bold, mType->getName(), fmt::defaults));
                    exit = true;
                }
            }
        }
    }

    bool GlobalVar::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}
