// Copyright 2025 JesusTouchMe

#include "Bible/parser/ast/global/Function.h"

namespace parser {
    Function::Function(std::vector<FunctionModifier> modifiers, std::string name, FunctionType* type,
                       std::vector<FunctionArgument> arguments, std::vector<ASTNodePtr> body, symbol::ScopePtr scope,
                       lexer::Token token)
                       : ASTNode(scope->parent, type, std::move(token))
                       , mModifiers(std::move(modifiers))
                       , mName(std::move(name))
                       , mArguments(std::move(arguments))
                       , mBody(std::move(body))
                       , mOwnScope(std::move(scope)) {
        u16 rawModifiers = 0;
        for (auto modifier : mModifiers) {
            rawModifiers |= static_cast<u16>(modifier);
        }

        mScope->createFunction(mName, static_cast<FunctionType*>(mType), rawModifiers & MODULEWEB_FUNCTION_MODIFIER_PUBLIC);

        u16 index = 0;

        for (auto& argument : mArguments) {
            mOwnScope->locals.emplace(argument.name, symbol::LocalSymbol(index, argument.type));
            index += argument.type->getStackSlots();
        }
    }

    void Function::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) {
        u16 modifiers = 0;
        for (auto modifier : mModifiers) {
            modifiers |= static_cast<u16>(modifier);
        }

        auto functionType = mType->getJesusASMType();
        auto function = builder.addFunction(modifiers, mName, functionType);

        if (mBody.empty()) {
            return;
        }

        function->attributes.push_back(std::make_unique<JesusASM::Attribute<bool>>("CompilerOptimized", true));

        builder.setInsertPoint(&function->instructions);

        for (auto& value : mBody) {
            value->codegen(builder, ctx, diag);
        }

        if (auto type = static_cast<FunctionType*>(mType); type->getReturnType()->isVoidType()) {
            builder.createReturn(type->getReturnType());
        }
    }

    void Function::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        for (auto& value : mBody) {
            value->semanticCheck(diag, exit, true);
        }
    }

    void Function::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        for (auto& value : mBody) {
            value->typeCheck(diag, exit);
        }
    }

    bool Function::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}