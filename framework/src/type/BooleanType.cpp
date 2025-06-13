// Copyright 2025 JesusTouchMe

#include "Bibble/type/BooleanType.h"

BooleanType::BooleanType()
    : Type("bool") {}

JesusASM::Type* BooleanType::getJesusASMType() const {
    return JesusASM::Type::GetBuiltinType("bool");
}

codegen::Type BooleanType::getRuntimeType() const {
    return codegen::Type::Primitive;
}

Type::CastLevel BooleanType::castTo(Type* destType) const {
    if (destType->isIntegerType() || destType->isCharType()) {
        return CastLevel::Implicit;
    }
    return Type::CastLevel::Disallowed;
}

bool BooleanType::isBooleanType() const {
    return true;
}