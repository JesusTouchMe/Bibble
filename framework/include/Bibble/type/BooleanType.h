// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_BOOLEANTYPE_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_BOOLEANTYPE_H

#include "Bibble/type/Type.h"

class BooleanType : public Type {
public:
    BooleanType();

    JesusASM::Type* getJesusASMType() const override;
    codegen::Type getRuntimeType() const override;

    CastLevel castTo(Type* destType) const override;

    bool isBooleanType() const override;
};

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_BOOLEANTYPE_H
