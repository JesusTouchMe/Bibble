// Copyright 2025 JesusTouchMe

#include "Bibble/type/HandleType.h"

HandleType::HandleType()
    : Type("handle") {}

JesusASM::Type* HandleType::getJesusASMType() const {
    return JesusASM::Type::GetBuiltinType("handle");
}

codegen::Type HandleType::getRuntimeType() const {
    return codegen::Type::Handle;
}

Type::CastLevel HandleType::castTo(Type* destType) const {
    return CastLevel::Disallowed;
}

bool HandleType::isHandleType() const {
    return true;
}