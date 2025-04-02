// Copyright 2025 JesusTouchMe

#include "Bible/type/HandleType.h"

HandleType::HandleType()
    : Type("handle") {}

int HandleType::getStackSlots() const {
    return 2;
}

JesusASM::Type* HandleType::getJesusASMType() const {
    return JesusASM::Type::GetBuiltinType("handle");
}

codegen::Type HandleType::getRuntimeType() const {
    return codegen::Type::Category2_Handle;
}

Type::CastLevel HandleType::castTo(Type* destType) const {
    return CastLevel::Disallowed;
}

bool HandleType::isHandleType() const {
    return true;
}