// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_HANDLETYPE_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_HANDLETYPE_H

#include "Bibble/type/Type.h"

class HandleType : public Type {
public:
    HandleType();

    int getStackSlots() const override;
    JesusASM::Type* getJesusASMType() const override;
    codegen::Type getRuntimeType() const override;

    CastLevel castTo(Type* destType) const override;

    bool isHandleType() const override;
};

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_HANDLETYPE_H
