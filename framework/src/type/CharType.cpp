// Copyright 2025 JesusTouchMe
#include "Bibble/type/CharType.h"
#include "Bibble/type/IntegerType.h"

CharType::CharType()
    : Type("char") {}

JesusASM::Type* CharType::getJesusASMType() const {
    return JesusASM::Type::GetBuiltinType("char");
}

codegen::Type CharType::getRuntimeType() const {
    return codegen::Type::Primitive;
}

Type::CastLevel CharType::castTo(Type* destType) const {
    if (destType->isBooleanType()) {
        return Type::CastLevel::Implicit;
    } else if (destType->isIntegerType()) {
        auto integerType = static_cast<IntegerType*>(destType);

        if (integerType->getSize() > IntegerType::Size::Short) {
            return Type::CastLevel::ImplicitWarning;
        }

        return Type::CastLevel::Implicit;
    }

    return Type::CastLevel::Disallowed;
}

bool CharType::isCharType() const {
    return true;
}