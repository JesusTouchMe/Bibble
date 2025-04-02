// Copyright 2025 JesusTouchMe
#include "Bible/type/CharType.h"
#include "Bible/type/IntegerType.h"

CharType::CharType()
    : Type("char") {}

int CharType::getStackSlots() const {
    return 1;
}

JesusASM::Type* CharType::getJesusASMType() const {
    return JesusASM::Type::GetBuiltinType("char");
}

codegen::Type CharType::getRuntimeType() const {
    return codegen::Type::Category1_Primitive;
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