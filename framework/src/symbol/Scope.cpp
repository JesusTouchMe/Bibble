// Copyright 2025 JesusTouchMe

#include "Bible/symbol/Scope.h"

#include <format>

namespace symbol {
    LocalSymbol::LocalSymbol(u16 index, Type* type)
        : index(index)
        , type(type) {}

    ClassSymbol::ClassSymbol(ClassType* type, std::vector<Field> fields, bool isPublic)
        : isPublic(isPublic)
        , type(type)
        , fields(std::move(fields)) {}

    FunctionSymbol::FunctionSymbol(std::string moduleName, std::string name, FunctionType* type, bool isPublic)
        : moduleName(std::move(moduleName))
        , name(std::move(name))
        , type(type)
        , isPublic(isPublic) {}

    Scope::Scope(Scope* parent, ClassSymbol* owner)
        : parent(parent)
        , owner(owner)
        , currentReturnType(nullptr) {}

    LocalSymbol* Scope::findLocal(std::string_view name) {
        Scope* scope = this;
        while (scope != nullptr) {
            auto it = scope->locals.find(name);
            if (it != scope->locals.end()) {
                return &it->second;
            }

            scope = scope->parent;
        }

        return nullptr;
    }

    ClassSymbol* Scope::findClass(std::string_view moduleName, std::string_view name) {
        std::string fullName = std::format("{}:{}", moduleName, name);
        auto it = classes.find(fullName);

        if (it != classes.end()) {
            return &it->second;
        }

        return nullptr;
    }

    ClassSymbol* Scope::findClass(ClassType* type) {
        return findClass(type->getModuleName(), type->getName());
    }

    FunctionSymbol* Scope::findFunction(std::string_view moduleName, std::string_view name, FunctionType* type) {
        std::string fullName = std::format("{}:{}:{}", moduleName, name, type->getName());
        auto it = functions.find(fullName);

        if (it != functions.end()) {
            return &it->second;
        }

        return nullptr;
    }

    ClassSymbol* Scope::findOwner() {
        Scope* scope = this;
        while (scope != nullptr) {
            if (scope->owner != nullptr) {
                return scope->owner;
            }

            scope = scope->parent;
        }

        return nullptr;
    }

    void Scope::createClass(ClassType* type, std::vector<ClassSymbol::Field> fields, bool isPublic) {
        std::string fullName = std::format("{}:{}", type->getModuleName(), type->getName());
        classes[fullName] = ClassSymbol(type, std::move(fields), isPublic);
    }

    void Scope::createFunction(std::string moduleName, std::string name, FunctionType* type, bool isPublic) {
        std::string fullName = std::format("{}:{}:{}", moduleName, name, type->getName());
        functions[fullName] = FunctionSymbol(moduleName, name, type, isPublic);
    }
}