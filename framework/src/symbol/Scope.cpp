// Copyright 2025 JesusTouchMe

#include "Bible/symbol/Scope.h"

#include <format>
#include <utility>

namespace symbol {
    LocalSymbol::LocalSymbol(u16 index, Type* type)
        : index(index)
        , type(type) {}

    ClassSymbol::ClassSymbol(ClassType* type, std::vector<Field> fields, bool isPublic)
        : isPublic(isPublic)
        , type(type)
        , fields(std::move(fields)) {}

    void ClassSymbol::Create(ClassType* type, std::vector<Field> fields, bool isPublic) {
        std::string name = std::format("{}:{}", type->getModuleName(), type->getName());

        GlobalClasses[name] = ClassSymbol(type, std::move(fields), isPublic);
    }

    FunctionSymbol::FunctionSymbol(std::string moduleName, std::string name, FunctionType* type, bool isPublic)
        : moduleName(std::move(moduleName))
        , name(std::move(name))
        , type(type)
        , isPublic(isPublic) {}

    void FunctionSymbol::Create(std::string moduleName, std::string name, FunctionType* type, bool isPublic) {
        std::string fullName = std::format("{}:{}:{}", moduleName, name, type->getName());

        GlobalFunctions[fullName] = FunctionSymbol(std::move(moduleName), std::move(name), type, isPublic);
    }

    ClassSymbol* FindClass(std::string_view moduleName, std::string_view name) {
        std::string fullName = std::format("{}:{}", moduleName, name);
        auto it = GlobalClasses.find(fullName);

        if (it != GlobalClasses.end()) {
            return &it->second;
        }

        return nullptr;
    }

    ClassSymbol* FindClass(ClassType* type) {
        return FindClass(type->getModuleName(), type->getName());
    }

    FunctionSymbol* FindFunction(std::string_view moduleName, std::string_view name, FunctionType* type) {
        std::string fullName = std::format("{}:{}:{}", moduleName, name, type->getName());
        auto it = GlobalFunctions.find(fullName);

        if (it != GlobalFunctions.end()) {
            return &it->second;
        }

        return nullptr;
    }

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
}