// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_SYMBOL_SCOPE_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_SYMBOL_SCOPE_H

#include "Bibble/type/ClassType.h"
#include "Bibble/type/FunctionType.h"

#include <unordered_map>

struct StringViewHash {
    using is_transparent = void;

    std::size_t operator()(std::string_view sv) const noexcept {
        return std::hash<std::string_view>{}(sv);
    }
};

struct StringViewEqual {
    using is_transparent = void;

    bool operator()(std::string_view lhs, std::string_view rhs) const noexcept {
        return lhs == rhs;
    }
};

namespace symbol {
    struct LocalSymbol {
        LocalSymbol() = default;
        LocalSymbol(u16 index, Type* type);

        u16 index;
        Type* type;
    };

    struct ClassSymbol {
        struct Field {
            u16 modifiers;
            std::string name;
            Type* type;
        };

        struct Method {
            u16 modifiers;
            std::string name;
            FunctionType* type;
        };

        ClassSymbol() = default;
        ClassSymbol(std::string moduleName, std::string name, std::vector<Field> fields, std::vector<Method> constructors, std::vector<Method> methods, bool isPublic);

        ClassType* getType() const;

        Field* getField(std::string_view name);
        Method* getMethod(std::string_view name);

        bool isPublic;
        std::string moduleName;
        std::string name;
        std::vector<Field> fields;
        std::vector<Method> constructors;
        std::vector<Method> methods;
    };

    struct FunctionSymbol {
        FunctionSymbol() = default;
        FunctionSymbol(std::string moduleName, std::string name, FunctionType* type, u16 modifiers);

        std::string moduleName;
        std::string name;
        u16 modifiers;
        FunctionType* type;
    };

    struct Scope {
        Scope(Scope* parent, std::string name, bool isGlobalScope, Type* currentReturnType = nullptr);

        std::vector<std::string> getNames();

        std::vector<FunctionSymbol*> getCandidateFunctions(std::vector<std::string> names);
        std::vector<FunctionSymbol*> getCandidateFunctionsDown(std::string name);
        std::vector<FunctionSymbol*> getCandidateFunctionsDown(std::vector<std::string> names);

        std::string_view findModuleName(std::string_view name);

        Scope* findModuleScope();

        LocalSymbol* findLocal(std::string_view name);
        ClassSymbol* findClass(std::string_view name);
        ClassSymbol* findClass(std::vector<std::string> names);
        FunctionSymbol* findFunction(std::string_view name, FunctionType* type);
        FunctionSymbol* findFunction(std::vector<std::string> names, FunctionType* type);
        ClassSymbol* findOwner();
        int* findVariableIndex();

        ClassSymbol* resolveClassSymbolDown(std::string_view name);
        ClassSymbol* resolveClassSymbolDown(std::vector<std::string> names);
        FunctionSymbol* resolveFunctionSymbolDown(std::string_view name, FunctionType* type);
        FunctionSymbol* resolveFunctionSymbolDown(std::vector<std::string> names, FunctionType* type);

        void createClass(std::string className, std::vector<ClassSymbol::Field> fields, std::vector<ClassSymbol::Method> constructors, std::vector<ClassSymbol::Method> methods, bool isPublic);
        void createFunction(std::string functionName, FunctionType* type, u16 modifiers);

        std::string name;

        bool isGlobalScope;

        Scope* parent;
        std::vector<Scope*> children;

        ClassSymbol* owner;
        Type* currentReturnType;

        int currentVariableIndex = -1;

        std::unordered_map<std::string, std::string, StringViewHash, StringViewEqual> importedModuleNames;

        std::unordered_map<std::string, LocalSymbol, StringViewHash, StringViewEqual> locals;
        std::unordered_map<std::string, ClassSymbol, StringViewHash, StringViewEqual> classes;
        std::vector<FunctionSymbol> functions;
    };

    using ScopePtr = std::unique_ptr<Scope>;
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_SYMBOL_SCOPE_H
