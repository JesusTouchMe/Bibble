// Copyright 2025 JesusTouchMe

#include "Bible/type/IntegerType.h"

IntegerType::IntegerType(std::string_view name, int stackSlots)
    : Type(name)
    , mStackSlots(stackSlots) {}

int IntegerType::getStackSlots() const {
    return mStackSlots;
}

JesusASM::Type* IntegerType::getJesusASMType() const {
    return JesusASM::Type::GetBuiltinType(mName);
}

bool IntegerType::isIntegerType() const {
    return true;
}