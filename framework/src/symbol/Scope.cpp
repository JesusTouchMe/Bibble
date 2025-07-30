// Copyright 2025 JesusTouchMe

#include "Bibble/symbol/Scope.h"

#include <algorithm>
#include <format>

namespace symbol {
    LocalSymbol::LocalSymbol(u16 index, Type* type)
        : index(index)
        , type(type) {}

    ClassSymbol::ClassSymbol(std::string moduleName, std::string name, ClassSymbol* baseClass, std::vector<Field> fields, std::vector<Method> constructors, std::vector<Method> methods, bool isPublic)
        : isPublic(isPublic)
        , moduleName(std::move(moduleName))
        , name(std::move(name))
        , baseClass(baseClass)
        , fields(std::move(fields))
        , constructors(std::move(constructors))
        , methods(std::move(methods)) {
        std::replace(this->moduleName.begin(), this->moduleName.end(), '.', '/');
    }

    ClassType* ClassSymbol::getType() const {
        return ClassType::Find(moduleName, name);
    }

    ClassSymbol::Field* ClassSymbol::getField(std::string_view name) {
        auto it = std::find_if(fields.begin(), fields.end(), [&name](const Field& field) {
           return field.name == name;
        });

        if (it != fields.end()) return &*it;
        else if (baseClass != nullptr) return baseClass->getField(name);
        else return nullptr;
    }

    ClassSymbol::Method* ClassSymbol::getMethod(std::string_view name, FunctionType* type) {
        auto it = std::find_if(methods.begin(), methods.end(), [name, type](const Method& method) {
            return method.name == name && (type == nullptr || method.languageType == type);
        });

        if (it != methods.end()) return &*it;
        else if (baseClass != nullptr) return baseClass->getMethod(name);
        else return nullptr;
    }

    std::vector<ClassSymbol::Method*> ClassSymbol::getCandidateMethods(std::string_view name) {
        std::vector<Method*> candidates;
        std::unordered_set<Signature> seen;
        getCandidateMethods(candidates, seen, name);
        return candidates;
    }

    bool ClassSymbol::getCandidateMethods(std::vector<Method*>& candidates, std::unordered_set<Signature>& seen, std::string_view name) {
        bool found = false;

        auto GetSignature = [](const Method& method) -> Signature {
            return Signature(method.name, method.languageType);
        };

        for (auto& method : methods) {
            if (method.name == name) {
                Signature signature = GetSignature(method);
                if (!seen.contains(signature)) {
                    seen.insert(signature);
                    candidates.push_back(&method);
                    found = true;
                }
            }
        }

        if (baseClass != nullptr) baseClass->getCandidateMethods(candidates, seen, name);

        return found;
    }


    FunctionSymbol::FunctionSymbol(std::string moduleName, std::string name, FunctionType* type, u16 modifiers)
        : moduleName(std::move(moduleName))
        , name(std::move(name))
        , type(type)
        , modifiers(modifiers) {
        std::replace(this->moduleName.begin(), this->moduleName.end(), '.', '/');
    }

    GlobalVarSymbol::GlobalVarSymbol(std::string moduleName, std::string name, u16 modifiers, Type* type)
        : moduleName(std::move(moduleName))
        , name(std::move(name))
        , modifiers(modifiers)
        , type(type) {
        std::replace(this->moduleName.begin(), this->moduleName.end(), '.', '/');
    }

    Scope::Scope(Scope* parent, std::string name, bool isGlobalScope, Type* currentReturnType)
        : name(std::move(name))
        , isGlobalScope(isGlobalScope)
        , parent(parent)
        , owner(nullptr)
        , currentReturnType(currentReturnType)
        , loopContext(nullptr, nullptr) {
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

        return nullptr;
    }

    codegen::Label* Scope::getBreakLabel(std::string name) {
        Scope* scope = this;
        while (scope != nullptr) {
            if (scope->loopContext.breakLabel != nullptr) {
                if (name.empty() || scope->loopContext.name == name) {
                    return scope->loopContext.breakLabel;
                }
            }

            scope = scope->parent;
        }

        return nullptr;
    }

    codegen::Label* Scope::getContinueLabel(std::string name) {
        Scope* scope = this;
        while (scope != nullptr) {
            if (scope->loopContext.continueLabel != nullptr) {
                if (name.empty() || scope->loopContext.name == name) {
                    return scope->loopContext.continueLabel;
                }
            }

            scope = scope->parent;
        }

        return nullptr;
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

        std::sort(candidateFunctions.begin(), candidateFunctions.end());
        candidateFunctions.erase(std::unique(candidateFunctions.begin(), candidateFunctions.end()), candidateFunctions.end());

        return candidateFunctions;
    }

    std::vector<FunctionSymbol*> Scope::getCandidateFunctionsDown(std::string name) {
        std::vector<FunctionSymbol*> candidateFunctions;

        for (auto& n : getNames()) {
            if (!n.empty()) return {};
        }

        auto it = std::find_if(functions.begin(), functions.end(), [&name](const auto& function) {
            return function->name == name;
        });

        while (it != functions.end()) {
            candidateFunctions.push_back(it->get());
            it = std::find_if(it + 1, functions.end(), [&name](const auto& function) {
                return function->name == name;
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
        std::vector<std::string> activeNames = getNames();
        std::erase(activeNames, "");

        if (std::equal(activeNames.begin(), activeNames.end(), names.begin(), names.end() - 1)) {
            auto it = std::find_if(functions.begin(), functions.end(), [&names](const auto& function) {
                return function->name == names.back();
            });

            while (it != functions.end()) {
                candidateFunctions.push_back(it->get());
                it = std::find_if(it + 1, functions.end(), [&names](const auto& function) {
                    return function->name == names.back();
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
                return symbol->name == name && (type == nullptr || symbol->type == type);
            });

            if (it != scope->functions.end()) {
                return it->get();
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

    GlobalVarSymbol* Scope::findGlobalVar(std::string_view name) {
        Scope* scope = this;
        while (scope != nullptr) {
            auto it = scope->globalVars.find(name);
            if (it != scope->globalVars.end()) {
                return &it->second;
            }

            scope = scope->parent;
        }

        if (auto sym = findModuleScope()->resolveGlobalVarSymbolDown(name)) return sym;
        return nullptr;
    }

    GlobalVarSymbol* Scope::findGlobalVar(std::vector<std::string> names) {
        std::vector<std::string> activeNames = getNames();

        Scope* moduleScope = findModuleScope();

        do {
            if (auto symbol = moduleScope->resolveGlobalVarSymbolDown(names)) return symbol;

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
            return symbol->name == name && (type == nullptr || symbol->type == type);
        });
        if (it != functions.end()) return it->get();

        for (auto child : children) {
            if (auto sym = child->resolveFunctionSymbolDown(name, type)) return sym;
        }

        return nullptr;
    }

    FunctionSymbol* Scope::resolveFunctionSymbolDown(std::vector<std::string> names, FunctionType* type) {
        std::vector<std::string> activeNames = getNames();

        if (std::equal(activeNames.begin(), activeNames.end(), names.begin(), names.end() - 1)) {
            auto it = std::find_if(functions.begin(), functions.end(), [&names, type](const auto& symbol) {
               return symbol->name == names.back() && (type == nullptr || symbol->type == type);
            });

            if (it != functions.end()) return it->get();
        }

        for (auto child : children) {
            if (auto symbol = child->resolveFunctionSymbolDown(names, type)) {
                return symbol;
            }
        }

        return nullptr;
    }

    GlobalVarSymbol* Scope::resolveGlobalVarSymbolDown(std::string_view name) {
        for (auto& n : getNames()) {
            if (!n.empty()) return nullptr;
        }

        auto it = globalVars.find(name);
        if (it != globalVars.end()) return &it->second;

        for (auto child : children) {
            if (auto sym = child->resolveGlobalVarSymbolDown(name)) return sym;
        }

        return nullptr;
    }

    GlobalVarSymbol* Scope::resolveGlobalVarSymbolDown(std::vector<std::string> names) {
        std::vector<std::string> activeNames = getNames();

        if (std::equal(activeNames.begin(), activeNames.end(), names.begin(), names.end() - 1)) {
            auto it = globalVars.find(names.back());
            if (it != globalVars.end()) return &it->second;
        }

        for (auto child : children) {
            if (auto symbol = child->resolveGlobalVarSymbolDown(names)) {
                return symbol;
            }
        }

        return nullptr;
    }

    void Scope::createClass(const std::string& className, ClassSymbol* baseClass, std::vector<ClassSymbol::Field> fields, std::vector<ClassSymbol::Method> constructors, std::vector<ClassSymbol::Method> methods, bool isPublic) {
        classes[className] = ClassSymbol(findModuleScope()->name, className, baseClass, std::move(fields), std::move(constructors), std::move(methods), isPublic);
    }

    FunctionSymbol* Scope::createFunction(std::string functionName, FunctionType* type, u16 modifiers) {
        functions.push_back(std::make_unique<FunctionSymbol>(findModuleScope()->name, std::move(functionName), type, modifiers));
        return functions.back().get();
    }

    void Scope::createGlobalVar(const std::string& name, Type* type, u16 modifiers) {
        globalVars[name] = GlobalVarSymbol(findModuleScope()->name, name, modifiers, type);
    }
}
