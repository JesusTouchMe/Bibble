// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_SYMBOL_SCOPE_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_SYMBOL_SCOPE_H

#include "Bibble/type/ClassType.h"
#include "Bibble/type/FunctionType.h"

#include <unordered_map>
#include <unordered_set>

#include "Bibble/codegen/Builder.h"

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
    struct Signature {
        std::string_view name;
        FunctionType* desc;

        bool operator==(const Signature& other) const {
            return other.name == name && other.desc == desc;
        }
    };
}

namespace std {
    template<>
    struct hash<symbol::Signature> {
        std::size_t operator()(const symbol::Signature& signature) const {
            std::size_t hash = std::hash<std::string_view>{}(signature.name);
            hash ^= std::hash<Type*>{}(signature.desc);

            return hash;
        }
    };
}

namespace symbol {
    struct FunctionSymbol;

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
            std::string realName; // ClassName::methodName
            FunctionType* languageType; // The type it looks to have in the language. e.g. void()
            FunctionType* type;

            FunctionSymbol* function;

            bool isVirtual;
        };

        ClassSymbol() = default;
        ClassSymbol(std::string moduleName, std::string name, ClassSymbol* baseClass, std::vector<Field> fields, std::vector<Method> constructors, std::vector<Method> methods, bool isPublic);

        ClassType* getType() const;

        Field* getField(std::string_view name);
        Method* getMethod(std::string_view name, FunctionType* type = nullptr);

        std::vector<Method*> getCandidateMethods(std::string_view name);
        bool getCandidateMethods(std::vector<Method*>& candidates, std::unordered_set<Signature>& seen, std::string_view name);

        bool isPublic;
        std::string moduleName;
        std::string name;
        ClassSymbol* baseClass;
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

        bool appendCallerLocation = false;
    };

    struct GlobalVarSymbol {
        GlobalVarSymbol() = default;
        GlobalVarSymbol(std::string moduleName, std::string name, u16 modifiers, Type* type);

        std::string moduleName;
        std::string name;
        u16 modifiers;
        Type* type;
    };

    struct LoopContext {
        LoopContext(codegen::Label* breakLabel, codegen::Label* continueLabel, std::string name = "")
            : breakLabel(breakLabel)
            , continueLabel(continueLabel)
            , name(std::move(name)) {}

        codegen::Label* breakLabel;
        codegen::Label* continueLabel;
        std::string name;
    };

    struct Scope {
        Scope(Scope* parent, std::string name, bool isGlobalScope, Type* currentReturnType = nullptr);

        std::vector<std::string> getNames();

        Type* getCurrentReturnType();
        codegen::Label* getBreakLabel(std::string name = "");
        codegen::Label* getContinueLabel(std::string name = "");

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
        GlobalVarSymbol* findGlobalVar(std::string_view name);
        GlobalVarSymbol* findGlobalVar(std::vector<std::string> names);
        ClassSymbol* findOwner();
        int* findVariableIndex();

        ClassSymbol* resolveClassSymbolDown(std::string_view name);
        ClassSymbol* resolveClassSymbolDown(std::vector<std::string> names);
        FunctionSymbol* resolveFunctionSymbolDown(std::string_view name, FunctionType* type);
        FunctionSymbol* resolveFunctionSymbolDown(std::vector<std::string> names, FunctionType* type);
        GlobalVarSymbol* resolveGlobalVarSymbolDown(std::string_view name);
        GlobalVarSymbol* resolveGlobalVarSymbolDown(std::vector<std::string> names);

        void createClass(const std::string& className, ClassSymbol* baseClass, std::vector<ClassSymbol::Field> fields, std::vector<ClassSymbol::Method> constructors, std::vector<ClassSymbol::Method> methods, bool isPublic);
        FunctionSymbol* createFunction(std::string functionName, FunctionType* type, u16 modifiers);
        void createGlobalVar(const std::string& name, Type* type, u16 modifiers);

        std::string name;

        bool isGlobalScope;

        Scope* parent;
        std::vector<Scope*> children;

        ClassSymbol* owner;
        Type* currentReturnType;
        LoopContext loopContext;

        int currentVariableIndex = -1;

        std::unordered_map<std::string, std::string, StringViewHash, StringViewEqual> importedModuleNames;

        std::unordered_map<std::string, LocalSymbol, StringViewHash, StringViewEqual> locals;
        std::unordered_map<std::string, ClassSymbol, StringViewHash, StringViewEqual> classes;
        std::vector<std::unique_ptr<FunctionSymbol>> functions;
        std::unordered_map<std::string, GlobalVarSymbol, StringViewHash, StringViewEqual> globalVars;
    };

    using ScopePtr = std::unique_ptr<Scope>;
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_SYMBOL_SCOPE_H
