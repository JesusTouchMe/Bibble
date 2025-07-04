// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/global/ClassDeclaration.h"

#include "Bibble/type/ViewType.h"

namespace parser {

    ClassField::ClassField(std::vector<FieldModifier> modifiers, Type* type, std::string name)
        : modifiers(std::move(modifiers))
        , type(type)
        , name(std::move(name)) {}

    ClassMethod::ClassMethod(std::vector<MethodModifier> modifiers, std::string name, FunctionType* type, std::vector<FunctionArgument> arguments,
                             std::vector<ASTNodePtr> body, symbol::ScopePtr scope, lexer::Token errorToken, bool overrides, bool viewSafe, bool isVirtual)
        : modifiers(std::move(modifiers))
        , name(std::move(name))
        , type(type)
        , arguments(std::move(arguments))
        , body(std::move(body))
        , scope(std::move(scope))
        , errorToken(std::move(errorToken))
        , overrides(overrides)
        , viewSafe(viewSafe)
        , isVirtual(isVirtual) {}

    ClassDeclaration::ClassDeclaration(std::vector<ClassModifier> modifiers, std::string name,
                                       std::vector<ClassField> fields, std::vector<ClassMethod> constructors, std::vector<ClassMethod> methods,
                                       symbol::ScopePtr scope, lexer::Token token)
                                       : ASTNode(scope->parent, nullptr, std::move(token))
                                       , mModifiers(std::move(modifiers))
                                       , mName(std::move(name))
                                       , mFields(std::move(fields))
                                       , mConstructors(std::move(constructors))
                                       , mMethods(std::move(methods))
                                       , mOwnScope(std::move(scope)) {
        ClassType* classType;
        mType = classType = ClassType::Find(mOwnScope->findModuleScope()->name, mName);

        classType = classType->getBaseType();

        symbol::ClassSymbol* baseClass = classType == nullptr ? nullptr : mScope->findClass({ std::string(classType->getModuleName()), std::string(classType->getName()) });

        std::vector<symbol::ClassSymbol::Field> fieldSymbols;
        std::vector<symbol::ClassSymbol::Method> constructorSymbols;
        std::vector<symbol::ClassSymbol::Method> methodSymbols;

        for (auto& field : mFields) {
            u16 fieldModifiers = 0;
            for (auto modifier : field.modifiers) {
                fieldModifiers |= static_cast<u16>(modifier);
            }

            fieldSymbols.push_back({ fieldModifiers, field.name, field.type });
        }

        if (mConstructors.empty()) {
            mConstructors.emplace_back(std::vector<MethodModifier>(), "#Init", FunctionType::Create(Type::Get("void"), {}),
                                       std::vector<FunctionArgument>(), std::vector<ASTNodePtr>(),
                                       std::make_unique<symbol::Scope>(mOwnScope.get(), "", false, Type::Get("void")),
                                       lexer::Token(), false, false, false);
            mConstructors.back().scope->currentVariableIndex = 0;
        }

        for (auto& method : mConstructors) {
            u16 methodModifiers = 0;
            for (auto modifier : method.modifiers) {
                methodModifiers |= static_cast<u16>(modifier);
            }

            auto languageType = method.type;

            auto argumentTypes = method.type->getArgumentTypes();
            argumentTypes.insert(argumentTypes.begin(), mType);
            method.type = FunctionType::Create(method.type->getReturnType(), std::move(argumentTypes));

            auto methodScope = method.scope->parent;
            symbol::FunctionSymbol* functionSymbol = methodScope->createFunction(mName + "::" + method.name, method.type, methodModifiers);

            method.arguments.insert(method.arguments.begin(), FunctionArgument(mType, "this"));

            int* index = method.scope->findVariableIndex();
            for (auto& argument : method.arguments) {
                method.scope->locals.emplace(argument.name, symbol::LocalSymbol(*index, argument.type));

                *index += 1;
            }

            constructorSymbols.push_back({ methodModifiers, method.name, mName + "::" + method.name, languageType, method.type, functionSymbol, false });
        }

        for (auto& method : mMethods) {
            u16 methodModifiers = 0;
            for (auto modifier : method.modifiers) {
                methodModifiers |= static_cast<u16>(modifier);

                if (modifier == MethodModifier::Abstract) method.isVirtual = true;
            }

            symbol::ClassSymbol::Method* foundMethod = nullptr;

            if (baseClass != nullptr) {
                symbol::ClassSymbol* current = baseClass;
                while (current != nullptr) {
                    foundMethod = current->getMethod(method.name, method.type);
                    if (foundMethod != nullptr) break;

                    current = current->baseClass;
                }
            }

            if (method.overrides) {
                if (foundMethod == nullptr) {
                    //TODO: error
                    int a = 5;
                }
            }

            if (!method.isVirtual && foundMethod != nullptr) {
                method.isVirtual = foundMethod->isVirtual;
            }

            auto languageType = method.type;

            Type* thisType = method.viewSafe ? ViewType::Create(mType) : mType;

            auto argumentTypes = method.type->getArgumentTypes();
            argumentTypes.insert(argumentTypes.begin(), thisType);
            method.type = FunctionType::Create(method.type->getReturnType(), std::move(argumentTypes));

            method.arguments.insert(method.arguments.begin(), FunctionArgument(thisType, "this"));

            auto methodScope = method.scope->parent;
            if (method.viewSafe) method.name += ".v";

            symbol::FunctionSymbol* functionSymbol = methodScope->createFunction(mName + "::" + method.name, method.type, methodModifiers);

            int* index = method.scope->findVariableIndex();
            for (auto& argument : method.arguments) {
                method.scope->locals.emplace(argument.name, symbol::LocalSymbol(*index, argument.type));

                *index += 1;
            }

            methodSymbols.push_back({ methodModifiers, method.name, mName + "::" + method.name, languageType, method.type, functionSymbol, method.isVirtual });
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

        if (baseClass == nullptr && classType != nullptr) {
            //TODO: error
            int a = 5;
        }

        mScope->createClass(mName, baseClass, std::move(fieldSymbols), std::move(constructorSymbols), std::move(methodSymbols), isPublic);
        mOwnScope->owner = mScope->findClass(mName);
    }

    void ClassDeclaration::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        u16 modifiers = 0;
        for (auto modifier : mModifiers) {
            modifiers |= static_cast<u16>(modifier);
        }

        auto classType = static_cast<ClassType*>(mType);

        JesusASM::tree::ClassNode* classNode;
        if (classType->getBaseType() != nullptr)
            classNode = builder.addClass(modifiers, mName, classType->getBaseType()->getModuleName(), classType->getBaseType()->getName());
        else
            classNode = builder.addClass(modifiers, mName);

        for (auto& field : mFields) {
            u16 fieldModifiers = 0;
            for (auto modifier : field.modifiers) {
                fieldModifiers |= static_cast<u16>(modifier);
            }

            classNode->fields.push_back(std::make_unique<JesusASM::tree::FieldNode>(fieldModifiers, field.name, field.type->getJesusASMType()->getDescriptor()));
        }

        for (auto& method : mConstructors) {
            u16 methodModifiers = 0;
            for (auto modifier : method.modifiers) {
                methodModifiers |= static_cast<u16>(modifier);
            }

            auto functionType = method.type->getJesusASMType();
            auto function = builder.addFunction(methodModifiers, mName + "::" + method.name, functionType);

            if (methodModifiers & MODULEWEB_FUNCTION_MODIFIER_NATIVE) {
                continue;
            }

            builder.setInsertPoint(&function->instructions);

            for (auto& value : method.body) {
                value->codegen(builder, ctx, diag, true);
            }

            if (auto type = static_cast<FunctionType*>(method.type); type->getReturnType()->isVoidType()) {
                builder.createReturn(type->getReturnType());
            }
        }

        for (auto& method : mMethods) {
            u16 methodModifiers = 0;
            for (auto modifier : method.modifiers) {
                methodModifiers |= static_cast<u16>(modifier);
            }

            auto functionType = method.type->getJesusASMType();
            auto function = builder.addFunction(methodModifiers, mName + "::" + method.name, functionType);

            if (methodModifiers & MODULEWEB_FUNCTION_MODIFIER_NATIVE) {
                continue;
            }

            builder.setInsertPoint(&function->instructions);

            for (auto& value : method.body) {
                value->codegen(builder, ctx, diag, true);
            }

            if (auto type = static_cast<FunctionType*>(method.type); type->getReturnType()->isVoidType()) {
                builder.createReturn(type->getReturnType());
            }

            if (method.isVirtual) {
                std::vector<::Type*> argTypes = method.type->getArgumentTypes();
                argTypes.erase(argTypes.begin());
                FunctionType* methodType = FunctionType::Create(method.type->getReturnType(), std::move(argTypes));

                classNode->methods.push_back(std::make_unique <JesusASM::tree::MethodNode>(
                    methodModifiers, method.name, std::string(methodType->getJesusASMType()->getDescriptor()),
                    JesusASM::tree::FunctionSymbol{ mScope->findModuleScope()->name, function->name, function->descriptor }));
            }
        }
    }

    void ClassDeclaration::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        for (auto& method : mConstructors) {
            for (auto& node : method.body) {
                node->semanticCheck(diag, exit, true);
            }
        }

        for (auto& method : mMethods) {
            for (auto& node : method.body) {
                node->semanticCheck(diag, exit, true);
            }
        }
    }

    void ClassDeclaration::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        for (auto& method : mConstructors) {
            for (auto& node : method.body) {
                node->typeCheck(diag, exit);
            }
        }

        for (auto& method : mMethods) {
            for (auto& node : method.body) {
                node->typeCheck(diag, exit);
            }
        }
    }

    bool ClassDeclaration::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}