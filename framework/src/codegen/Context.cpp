// Copyright 2025 JesusTouchMe

#include "Bibble/codegen/Context.h"

#include <cassert>

namespace codegen {
    Context::Context(std::unique_ptr<ModuleNode> module)
        : mModule(std::move(module)) {}

    ModuleNode* Context::getModule() {
        return mModule.get();
    }

    void Context::push(Value value) {
        mVirtualStack.push_back(value);
    }

    Value Context::pop() {
        assert(!mVirtualStack.empty());
        Value value = mVirtualStack.back();
        mVirtualStack.pop_back();
        return value;
    }

    Value& Context::top() {
        assert(!mVirtualStack.empty());
        return mVirtualStack.back();
    }
}