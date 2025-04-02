// Copyright 2025 JesusTouchMe

#include "Bibble/type/VoidType.h"

#include <iostream>

VoidType::VoidType()
    : Type("void") {}

int VoidType::getStackSlots() const {
    return 0;
}

JesusASM::Type* VoidType::getJesusASMType() const {
    return JesusASM::Type::GetBuiltinType("void");
}

codegen::Type VoidType::getRuntimeType() const {
    std::cerr << "bible: void type as a runtime value type\n";
    std::exit(1);
}

Type::CastLevel VoidType::castTo(Type* destType) const {
    return CastLevel::Disallowed;
}

bool VoidType::isVoidType() const {
    return true;
}