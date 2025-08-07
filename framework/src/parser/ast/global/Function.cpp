// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/global/Function.h"

namespace parser {
    Function::Function(std::vector<FunctionModifier> modifiers, std::string name, FunctionType* type,
                       std::vector<FunctionArgument> arguments, bool callerLocation, std::vector<ASTNodePtr> body, symbol::ScopePtr scope,
                       lexer::Token token)
                       : ASTNode(scope->parent, type, std::move(token))
                       , mModifiers(std::move(modifiers))
                       , mName(std::move(name))
                       , mArguments(std::move(arguments))
                       , mCallerLocation(callerLocation)
                       , mBody(std::move(body))
                       , mOwnScope(std::move(scope)) {
        u16 rawModifiers = 0;
        for (auto modifier : mModifiers) {
            rawModifiers |= static_cast<u16>(modifier);
        }

        auto* symbol = mScope->createFunction(mName, static_cast<FunctionType*>(mType), rawModifiers);
        symbol->appendCallerLocation = callerLocation;

        int* index = mOwnScope->findVariableIndex();

        for (auto& argument : mArguments) {
            mOwnScope->locals.emplace(argument.name, symbol::LocalSymbol(*index, argument.type));
            *index += 1;
        }
    }

    void Function::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        u16 modifiers = 0;
        for (auto modifier : mModifiers) {
            modifiers |= static_cast<u16>(modifier);
        }

        auto functionType = mType->getJesusASMType();
        auto function = builder.addFunction(modifiers, mName, functionType);

        if (mBody.empty()) {
            return;
        }

        function->attributes.push_back(std::make_unique<JesusASM::Attribute<bool>>("CompilerOptimized", false));

        builder.setInsertPoint(&function->instructions);

        for (auto& value : mBody) {
            value->codegen(builder, ctx, diag, true);
        }

        if (!function->instructions.hasTerminator()) {
            auto returnType = static_cast<FunctionType*>(mType)->getReturnType();
            if (returnType->isVoidType()) {
                builder.createReturn(returnType);
            } else {
                diag.compilerError(mBody.back()->getErrorToken().getStartLocation(),
                                   mBody.back()->getErrorToken().getEndLocation(),
                                   "missing return statement for non-void function");
                std::exit(1);
            }
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