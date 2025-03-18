// Copyright 2025 JesusTouchMe

#include "Bible/type/BooleanType.h"

BooleanType::BooleanType()
    : Type("bool") {}

int BooleanType::getStackSlots() const {
    return 1;
}

JesusASM::Type* BooleanType::getJesusASMType() const {
    return JesusASM::Type::GetBuiltinType("bool");
}

bool BooleanType::isBooleanType() const {
    return true;
}