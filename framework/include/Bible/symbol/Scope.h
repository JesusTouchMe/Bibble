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

        ClassSymbol() = default;
        ClassSymbol(ClassType* type, std::vector<Field> fields, bool isPublic);

        bool isPublic;
        ClassType* type;
        std::vector<Field> fields;
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
        Scope(Scope* parent, ClassSymbol* owner);

        LocalSymbol* findLocal(std::string_view name);
        ClassSymbol* findClass(std::string_view moduleName, std::string_view name);
        ClassSymbol* findClass(ClassType* type);
        FunctionSymbol* findFunction(std::string_view moduleName, std::string_view name, FunctionType* type);
        ClassSymbol* findOwner();

        void createClass(ClassType* type, std::vector<ClassSymbol::Field> fields, bool isPublic);
        void createFunction(std::string moduleName, std::string name, FunctionType* type, bool isPublic);

        std::unordered_map<std::string, LocalSymbol, StringViewHash, StringViewEqual> locals;
        std::unordered_map<std::string, ClassSymbol> classes;
        std::unordered_map<std::string, FunctionSymbol> functions;

        Scope* parent;
        ClassSymbol* owner;
        Type* currentReturnType;
    };

    using ScopePtr = std::unique_ptr<Scope>;
}

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_SYMBOL_SCOPE_H
