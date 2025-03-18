// Copyright 2025 JesusTouchMe

#include "Bible/type/VoidType.h"

VoidType::VoidType()
    : Type("void") {}

int VoidType::getStackSlots() const {
    return 0;
}

JesusASM::Type* VoidType::getJesusASMType() const {
    return JesusASM::Type::GetBuiltinType("void");
}

Type::CastLevel VoidType::castTo(Type* destType) const {
    return CastLevel::Disallowed;
}

bool VoidType::isVoidType() const {
    return true;
}