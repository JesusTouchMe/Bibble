// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_CONTEXT_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_CONTEXT_H

#include "Bibble/codegen/Type.h"

#include <JesusASM/tree/ModuleNode.h>

#include <optional>

namespace codegen {
    class Builder;

    using namespace JesusASM::tree;

    enum class CmpOperator {
        EQ, NE,
        LT, GT,
        LE, GE
    };

    struct ValueOrigin {
        ValueOrigin(AbstractInsnNode* origin, i64 value) : origin(origin), value(value) {}

        AbstractInsnNode* origin;
        i64 value;
    };

    struct Value {
        explicit Value(Type type, std::optional<ValueOrigin> value = std::nullopt)
            : type(type)
            , value(value) {}

        explicit Value(std::optional<CmpOperator> cmp, std::optional<ValueOrigin> value = std::nullopt)
            : type(Type::Compiler_CmpResult)
            , cmpOperator(cmp)
            , value(value) {}

        Type type;
        std::optional<CmpOperator> cmpOperator;
        std::optional<ValueOrigin> value; // only present if the value is 100% known
    };

    class Context {
    friend class Builder;
    public:
        explicit Context(std::unique_ptr<ModuleNode> module);

        ModuleNode* getModule();

        void push(Value value);

        template <typename... Args>
        constexpr void emplace(Args&&... args) {
            static_assert(std::is_constructible_v<Value, Args...>);
            push(Value(std::forward<Args>(args)...));
        }

        Value pop();

        Value& top();

    private:
        std::unique_ptr<ModuleNode> mModule;
        std::vector<Value> mVirtualStack;
    };
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_CONTEXT_H
