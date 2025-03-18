// Copyright 2025 JesusTouchMe
#include "Bible/type/CharType.h"

CharType::CharType()
    : Type("char") {}

int CharType::getStackSlots() const {
    return 1;
}

JesusASM::Type* CharType::getJesusASMType() const {
    return JesusASM::Type::GetBuiltinType("char");
}

bool CharType::isCharType() const {
    return true;
}
