// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/global/ClassDeclaration.h"

namespace parser {

    ClassField::ClassField(std::vector<FieldModifier> modifiers, Type* type, std::string name)
        : modifiers(std::move(modifiers))
        , type(type)
        , name(std::move(name)) {}

    ClassDeclaration::ClassDeclaration(std::vector<ClassModifier> modifiers, std::string name,
                                       std::vector<ClassField> fields, std::vector<ClassMethod> methods,
                                       symbol::ScopePtr scope, lexer::Token token)
                                       : ASTNode(scope->parent, nullptr, std::move(token))
                                       , mModifiers(std::move(modifiers))
                                       , mName(std::move(name))
                                       , mFields(std::move(fields))
                                       , mMethods(std::move(methods))
                                       , mOwnScope(std::move(scope)) {
        mType = ClassType::Create(mOwnScope->getNames().front(), name);

        std::vector<symbol::ClassSymbol::Field> fieldSymbols;
        std::vector<symbol::ClassSymbol::Method> methodSymbols;

        for (auto& field : mFields) {
            u16 fieldModifiers = 0;
            for (auto modifier : field.modifiers) {
                fieldModifiers |= static_cast<u16>(modifier);
            }

            fieldSymbols.push_back({ fieldModifiers, field.name, field.type });
        }

        for (auto& method : mMethods) {
            u16 methodModifiers = 0;
            for (auto modifier : method.modifiers) {
                methodModifiers |= static_cast<u16>(modifier);
            }

            auto argumentTypes = method.type->getArgumentTypes();
            argumentTypes.insert(argumentTypes.begin(), mType);
            method.type = FunctionType::Create(method.type->getReturnType(), std::move(argumentTypes));

            auto methodScope = method.scope->parent;
            methodScope->createFunction(method.name, method.type, methodModifiers & MODULEWEB_FUNCTION_MODIFIER_PUBLIC);

            method.arguments.insert(method.arguments.begin(), FunctionArgument(mType, "this"));

            u16 index = 0;
            for (auto& argument : method.arguments) {
                method.scope->locals.emplace(argument.name, symbol::LocalSymbol(index, argument.type));

                index += argument.type->getStackSlots();
            }

            methodSymbols.push_back({ methodModifiers, method.name, method.type });
        }

        bool isPublic = false;
        for (auto modifier : mModifiers) {
            if (modifier == ClassModifier::Public) {
                isPublic = true;
                break;
            } else if (modifier == ClassModifier::Private) {
                break;
            }
        }

        mScope->createClass(mName, std::move(fieldSymbols), std::move(methodSymbols), isPublic);
    }

    void ClassDeclaration::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) {
        u16 modifiers = 0;
        for (auto modifier : mModifiers) {
            modifiers |= static_cast<u16>(modifier);
        }

        auto classNode = builder.addClass(modifiers, mName);

        for (auto& field : mFields) {
            u16 fieldModifiers = 0;
            for (auto modifier : field.modifiers) {
                fieldModifiers |= static_cast<u16>(modifier);
            }

            classNode->fields.push_back(std::make_unique<JesusASM::tree::FieldNode>(fieldModifiers, field.name, field.type->getJesusASMType()->getDescriptor()));
        }

        for (auto& method : mMethods) {
            u16 methodModifiers = 0;
            for (auto modifier : method.modifiers) {
                methodModifiers |= static_cast<u16>(modifier);
            }

            auto functionType = method.type->getJesusASMType();
            auto function = builder.addFunction(methodModifiers, method.name, functionType);

            if (method.body.empty()) {
                continue;
            }

            builder.setInsertPoint(&function->instructions);

            for (auto& value : method.body) {
                value->codegen(builder, ctx, diag);
            }
        }
    }

    void ClassDeclaration::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {

    }

    void ClassDeclaration::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {

    }

    bool ClassDeclaration::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}