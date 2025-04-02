// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_VOIDTYPE_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_VOIDTYPE_H

#include "Bibble/type/Type.h"

class VoidType : public Type {
public:
    VoidType();

    int getStackSlots() const override;
    JesusASM::Type* getJesusASMType() const override;

    codegen::Type getRuntimeType() const override;

    CastLevel castTo(Type* destType) const override;

    bool isVoidType() const override;
};

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_VOIDTYPE_H
