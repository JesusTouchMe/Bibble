// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_CHARTYPE_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_CHARTYPE_H

#include "Bibble/type/Type.h"

class CharType : public Type {
public:
    CharType();

    int getStackSlots() const override;
    JesusASM::Type* getJesusASMType() const override;
    codegen::Type getRuntimeType() const override;

    CastLevel castTo(Type* destType) const override;

    bool isCharType() const override;
};

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_CHARTYPE_H
