// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_VOIDTYPE_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_VOIDTYPE_H

#include "Bible/type/Type.h"

class VoidType : public Type {
public:
    VoidType();

    int getStackSlots() const override;
    JesusASM::Type* getJesusASMType() const override;

    CastLevel castTo(Type* destType) const override;

    bool isVoidType() const override;
};

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_VOIDTYPE_H
