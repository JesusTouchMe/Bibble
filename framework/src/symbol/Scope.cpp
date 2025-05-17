// Copyright 2025 JesusTouchMe

#include "Bibble/symbol/Scope.h"

#include <algorithm>
#include <format>

namespace symbol {
    LocalSymbol::LocalSymbol(u16 index, Type* type)
        : index(index)
        , type(type) {}

    ClassSymbol::ClassSymbol(std::string moduleName, std::string name, std::vector<Field> fields, std::vector<Method> constructors, std::vector<Method> methods, bool isPublic)
        : isPublic(isPublic)
        , moduleName(std::move(moduleName))
        , name(std::move(name))
        , fields(std::move(fields))
        , constructors(std::move(constructors))
        , methods(std::move(methods)) {
        std::replace(moduleName.begin(), moduleName.end(), '.', '/');
    }

    ClassType* ClassSymbol::getType() const {
        return ClassType::Create(moduleName, name);
    }

    ClassSymbol::Field* ClassSymbol::getField(std::string_view name) {
        auto it = std::find_if(fields.begin(), fields.end(), [&name](const Field& field) {
           return field.name == name;
        });

        if (it != fields.end()) return &*it;
        return nullptr;
    }

    ClassSymbol::Method* ClassSymbol::getMethod(std::string_view name) {
        auto it = std::find_if(methods.begin(), methods.end(), [&name](const Method& method) {
            return method.name == name;
        });

        if (it != methods.end()) return &*it;
        return nullptr;
    }

    FunctionSymbol::FunctionSymbol(std::string moduleName, std::string name, FunctionType* type, u16 modifiers)
        : moduleName(std::move(moduleName))
        , name(std::move(name))
        , type(type)
        , modifiers(modifiers) {
        std::replace(moduleName.begin(), moduleName.end(), '.', '/');
    }

    Scope::Scope(Scope* parent, std::string name, bool isGlobalScope, Type* currentReturnType)
        : name(std::move(name))
        , isGlobalScope(isGlobalScope)
        , parent(parent)
        , owner(nullptr)
        , currentReturnType(currentReturnType) {
        if (parent != nullptr && isGlobalScope) {
            parent->children.push_back(this);
        }
    }

    std::vector<std::string> Scope::getNames() {
        std::vector<std::string> names;
        Scope* scope = this;

        while (scope != nullptr) {
            names.push_back(scope->name);
            scope = scope->parent;
        }

        std::reverse(names.begin(), names.end());
        return names;
    }

    Type* Scope::getCurrentReturnType() {
        Scope* scope = this;
        while (scope != nullptr) {
            if (scope->currentReturnType != nullptr) return scope->currentReturnType;
            scope = scope->parent;
        }
    }

    std::vector<FunctionSymbol*> Scope::getCandidateFunctions(std::vector<std::string> names) {
        std::vector<std::string> activeNames = getNames();
        std::vector<FunctionSymbol*> candidateFunctions;

        Scope* moduleScope = findModuleScope();

        do {
            auto candidates = moduleScope->getCandidateFunctionsDown(names);
            std::copy(candidates.begin(), candidates.end(), std::back_inserter(candidateFunctions));

            if (activeNames.empty()) break;

            names.insert(names.begin(), std::move(activeNames.back()));
            activeNames.erase(activeNames.end() - 1);
        } while (!activeNames.empty());

        return candidateFunctions;
    }

    std::vector<FunctionSymbol*> Scope::getCandidateFunctionsDown(std::string name) {
        std::vector<FunctionSymbol*> candidateFunctions;

        for (auto& n : getNames()) {
            if (!n.empty()) return {};
        }

        auto it = std::find_if(functions.begin(), functions.end(), [&name](const FunctionSymbol& function) {
            return function.name == name;
        });

        while (it != functions.end()) {
            candidateFunctions.push_back(&*it);
            it = std::find_if(it + 1, functions.end(), [&name](const FunctionSymbol& function) {
                return function.name == name;
            });
        }

        for (auto child : children) {
            auto childCandidateFunctions = child->getCandidateFunctionsDown(name);
            std::copy(childCandidateFunctions.begin(), childCandidateFunctions.end(), std::back_inserter(candidateFunctions));
        }

        return candidateFunctions;
    }

    std::vector<FunctionSymbol*> Scope::getCandidateFunctionsDown(std::vector<std::string> names) {
        std::vector<FunctionSymbol*> candidateFunctions;
        auto activeNames = getNames();
        std::erase(activeNames, "");

        if (std::equal(activeNames.begin(), activeNames.end(), names.begin(), names.end() - 1)) {
            auto it = std::find_if(functions.begin(), functions.end(), [&names](const FunctionSymbol& function) {
                return function.name == names.back();
            });

            while (it != functions.end()) {
                candidateFunctions.push_back(&*it);
                it = std::find_if(it + 1, functions.end(), [&names](const FunctionSymbol& function) {
                    return function.name == names.back();
                });
            }
        }

        for (auto child : children) {
            auto childCandidateFunctions = child->getCandidateFunctionsDown(names);
            std::copy(childCandidateFunctions.begin(), childCandidateFunctions.end(), std::back_inserter(candidateFunctions));
        }

        return candidateFunctions;
    }

    std::string_view Scope::findModuleName(std::string_view name) {
        Scope* scope = this;
        while (scope != nullptr) {
            auto it = scope->importedModuleNames.find(name);
            if (it != scope->importedModuleNames.end()) {
                return it->second;
            }

            scope = scope->parent;
        }

        return name;
    }

    Scope* Scope::findModuleScope() {
        Scope* scope = this;
        Scope* moduleScope = nullptr;

        while (scope != nullptr) {
            if (!scope->name.empty()) moduleScope = scope;
            scope = scope->parent;
        }

        return moduleScope;
    }

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

    ClassSymbol* Scope::findClass(std::string_view name) {
        Scope* scope = this;
        while (scope != nullptr) {
            auto it = scope->classes.find(name);
            if (it != scope->classes.end()) {
                return &it->second;
            }

            scope = scope->parent;
        }

        if (auto sym = findModuleScope()->resolveClassSymbolDown(name)) return sym;
        return nullptr;
    }

    ClassSymbol* Scope::findClass(std::vector<std::string> names) {
        std::vector<std::string> activeNames = getNames();

        Scope* moduleScope = findModuleScope();

        do {
            if (auto symbol = moduleScope->resolveClassSymbolDown(names)) return symbol;

            if (activeNames.empty()) break;

            names.insert(names.begin(), std::move(activeNames.back()));
            activeNames.erase(activeNames.end() - 1);
        } while (!activeNames.empty());

        return nullptr;
    }

    FunctionSymbol* Scope::findFunction(std::string_view name, FunctionType* type) {
        Scope* scope = this;
        while (scope != nullptr) {
            auto it = std::find_if(scope->functions.begin(), scope->functions.end(), [&name, type](const auto& symbol) {
                return symbol.name == name && (type == nullptr || symbol.type == type);
            });

            if (it != functions.end()) {
                return &*it;
            }

            scope = scope->parent;
        }

        if (auto sym = findModuleScope()->resolveFunctionSymbolDown(name, type)) return sym;
        return nullptr;
    }

    FunctionSymbol* Scope::findFunction(std::vector<std::string> names, FunctionType* type) {
        std::vector<std::string> activeNames = getNames();

        Scope* moduleScope = findModuleScope();

        do {
            if (auto symbol = moduleScope->resolveFunctionSymbolDown(names, type)) return symbol;

            if (activeNames.empty()) break;

            names.insert(names.begin(), std::move(activeNames.back()));
            activeNames.erase(activeNames.end() - 1);
        } while (!activeNames.empty());

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

    int* Scope::findVariableIndex() {
        Scope* scope = this;
        while (scope != nullptr) {
            if (scope->currentVariableIndex > -1) {
                return &scope->currentVariableIndex;
            }

            scope = scope->parent;
        }

        return nullptr;
    }

    ClassSymbol* Scope::resolveClassSymbolDown(std::string_view name) {
        for (auto& n : getNames()) {
            if (!n.empty()) return nullptr;
        }

        auto it = classes.find(name);
        if (it != classes.end()) return &it->second;

        for (auto child : children) {
            if (auto sym = child->resolveClassSymbolDown(name)) return sym;
        }

        return nullptr;
    }

    ClassSymbol* Scope::resolveClassSymbolDown(std::vector<std::string> names) {
        std::vector<std::string> activeNames = getNames();

        if (std::equal(activeNames.begin(), activeNames.end(), names.begin(), names.end() - 1)) {
            auto it = classes.find(names.back());
            if (it != classes.end()) return &it->second;
        }

        for (auto child : children) {
            if (auto symbol = child->resolveClassSymbolDown(names)) {
                return symbol;
            }
        }

        return nullptr;
    }

    FunctionSymbol* Scope::resolveFunctionSymbolDown(std::string_view name, FunctionType* type) {
        for (auto& n : getNames()) {
            if (!n.empty()) return nullptr;
        }

        auto it = std::find_if(functions.begin(), functions.end(), [name, type](const auto& symbol) {
            return symbol.name == name && (type == nullptr || symbol.type == type);
        });
        if (it != functions.end()) return &*it;

        for (auto child : children) {
            if (auto sym = child->resolveFunctionSymbolDown(name, type)) return sym;
        }

        return nullptr;
    }

    FunctionSymbol* Scope::resolveFunctionSymbolDown(std::vector<std::string> names, FunctionType* type) {
        std::vector<std::string> activeNames = getNames();

        if (std::equal(activeNames.begin(), activeNames.end(), names.begin(), names.end() - 1)) {
            auto it = std::find_if(functions.begin(), functions.end(), [&names, type](const auto& symbol) {
               return symbol.name == names.back() && (type == nullptr || symbol.type == type);
            });

            if (it != functions.end()) return &*it;
        }

        for (auto child : children) {
            if (auto symbol = child->resolveFunctionSymbolDown(names, type)) {
                return symbol;
            }
        }

        return nullptr;
    }

    void Scope::createClass(std::string className, std::vector<ClassSymbol::Field> fields, std::vector<ClassSymbol::Method> constructors, std::vector<ClassSymbol::Method> methods, bool isPublic) {
        classes[className] = ClassSymbol(findModuleScope()->name, className, std::move(fields), std::move(constructors), std::move(methods), isPublic);
    }

    void Scope::createFunction(std::string functionName, FunctionType* type, u16 modifiers) {
        functions.emplace_back(findModuleScope()->name, std::move(functionName), type, modifiers);
    }
}