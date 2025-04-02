// Copyright 2025 JesusTouchMe

#include "Bible/type/FunctionType.h"

#include <algorithm>
#include <format>

FunctionType::FunctionType(Type* returnType, std::vector<Type*> arguments)
    : Type(std::format("{}(", returnType->getName()))
    , mReturnType(returnType)
    , mArguments(std::move(arguments)) {
    if (!mArguments.empty()) {
        for (std::size_t i = 0; i < mArguments.size() - 1; i++) {
            mName += std::format("{}, ", mArguments[i]->getName());
        }
        mName += std::format("{})", mArguments.back()->getName());
    } else {
        mName += ')';
    }
}

Type* FunctionType::getReturnType() const {
    return mReturnType;
}

const std::vector<Type*>& FunctionType::getArgumentTypes() const {
    return mArguments;
}

int FunctionType::getStackSlots() const {
    return 2; // it's represented as a handle (eventually)
              // we'll add a 'calldynamic' instruction that takes a function type descriptor as its arg and treats the function at the handle as such
}

JesusASM::Type* FunctionType::getJesusASMType() const {
    std::vector<JesusASM::Type*> arguments;
    arguments.reserve(mArguments.size());

    for (auto arg : mArguments) {
        arguments.push_back(arg->getJesusASMType());
    }

    return JesusASM::Type::GetFunctionType(mReturnType->getJesusASMType(), arguments);
}

codegen::Type FunctionType::getRuntimeType() const {
    return codegen::Type::Category2_Handle;
}

Type::CastLevel FunctionType::castTo(Type* destType) const {
    return CastLevel::Disallowed;
}

bool FunctionType::isFunctionType() const {
    return true;
}

FunctionType* FunctionType::Create(Type* returnType, std::vector<Type*> arguments) {
    static std::vector<std::unique_ptr<FunctionType>> functionTypes;
    auto it = std::find_if(functionTypes.begin(), functionTypes.end(), [returnType, &arguments](const auto& type) {
        return type->getReturnType() == returnType && type->getArgumentTypes() == arguments;
    });

    if (it != functionTypes.end()) {
        return it->get();
    }

    functionTypes.push_back(std::make_unique<FunctionType>(returnType, std::move(arguments)));
    return functionTypes.back().get();
}

