// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_BOOLEANTYPE_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_BOOLEANTYPE_H

#include "Bible/type/Type.h"

class BooleanType : public Type {
public:
    BooleanType();

    int getStackSlots() const override;
    JesusASM::Type* getJesusASMType() const override;

    CastLevel castTo(Type* destType) const override;

    bool isBooleanType() const override;
};

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_BOOLEANTYPE_H
