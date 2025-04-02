// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_SYMBOL_SCOPE_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_SYMBOL_SCOPE_H

#include "Bible/type/ClassType.h"
#include "Bible/type/FunctionType.h"

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
        ClassSymbol(std::string moduleName, std::string name, std::vector<Field> fields, std::vector<Method> methods, bool isPublic);

        ClassType* getType() const;

        Field* getField(std::string_view name);
        Method* getMethod(std::string_view name);

        bool isPublic;
        std::string moduleName;
        std::string name;
        std::vector<Field> fields;
        std::vector<Method> methods;
    };

    struct FunctionSymbol {
        FunctionSymbol() = default;
        FunctionSymbol(std::string moduleName, std::string name, FunctionType* type, bool isPublic);

        std::string moduleName;
        std::string name;
        bool isPublic;
        FunctionType* type;
    };

    struct Scope {
        Scope(Scope* parent, std::string name, bool isGlobalScope, Type* currentReturnType = nullptr);

        std::vector<std::string> getNames();
        Scope* getTopLevelScope();

        std::vector<FunctionSymbol*> getCandidateFunctions(std::vector<std::string> names);
        std::vector<FunctionSymbol*> getCandidateFunctionsDown(std::string name);
        std::vector<FunctionSymbol*> getCandidateFunctionsDown(std::vector<std::string> names);

        std::string_view findModuleName(std::string_view name);

        LocalSymbol* findLocal(std::string_view name);
        ClassSymbol* findClass(std::string_view name);
        ClassSymbol* findClass(std::vector<std::string> names);
        FunctionSymbol* findFunction(std::string_view name, FunctionType* type);
        FunctionSymbol* findFunction(std::vector<std::string> names, FunctionType* type);
        ClassSymbol* findOwner();

        ClassSymbol* resolveClassSymbolDown(std::string_view name);
        ClassSymbol* resolveClassSymbolDown(std::vector<std::string> names);
        FunctionSymbol* resolveFunctionSymbolDown(std::string_view name, FunctionType* type);
        FunctionSymbol* resolveFunctionSymbolDown(std::vector<std::string> names, FunctionType* type);

        void createClass(std::string className, std::vector<ClassSymbol::Field> fields, std::vector<ClassSymbol::Method> methods, bool isPublic);
        void createFunction(std::string functionName, FunctionType* type, bool isPublic);

        std::string name;

        bool isGlobalScope;

        Scope* parent;
        std::vector<Scope*> children;

        ClassSymbol* owner;
        Type* currentReturnType;

        std::unordered_map<std::string, std::string, StringViewHash, StringViewEqual> importedModuleNames;

        std::unordered_map<std::string, LocalSymbol, StringViewHash, StringViewEqual> locals;
        std::unordered_map<std::string, ClassSymbol, StringViewHash, StringViewEqual> classes;
        std::vector<FunctionSymbol> functions;
    };

    using ScopePtr = std::unique_ptr<Scope>;
}

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_SYMBOL_SCOPE_H
