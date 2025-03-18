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

        static void Create(ClassType* type, std::vector<Field> fields, bool isPublic);
    };

    struct FunctionSymbol {
        FunctionSymbol() = default;
        FunctionSymbol(std::string moduleName, std::string name, FunctionType* type, bool isPublic);

        std::string moduleName;
        std::string name;
        bool isPublic;
        FunctionType* type;

        static void Create(std::string moduleName, std::string name, FunctionType* type, bool isPublic);
    };

    extern std::unordered_map<std::string, ClassSymbol> GlobalClasses;
    extern std::unordered_map<std::string, FunctionSymbol> GlobalFunctions;

    ClassSymbol* FindClass(std::string_view moduleName, std::string_view name);
    ClassSymbol* FindClass(ClassType* type);
    FunctionSymbol* FindFunction(std::string_view moduleName, std::string_view name, FunctionType* type);

    struct Scope {
        Scope(Scope* parent, ClassSymbol* owner);

        LocalSymbol* findLocal(std::string_view name);
        ClassSymbol* findOwner();

        std::unordered_map<std::string, LocalSymbol, StringViewHash, StringViewEqual> locals;

        Scope* parent;
        ClassSymbol* owner;
        Type* currentReturnType;
    };

    using ScopePtr = std::unique_ptr<Scope>;
}

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_SYMBOL_SCOPE_H
