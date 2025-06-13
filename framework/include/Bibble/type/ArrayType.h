// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_ARRAYTYPE_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_ARRAYTYPE_H 1

#include "Bibble/type/Type.h"

class ArrayType : public Type {
public:
    ArrayType(Type* elementType);

    Type* getElementType() const;

    JesusASM::Type* getJesusASMType() const override;
    codegen::Type getRuntimeType() const override;

    CastLevel castTo(Type* destType) const override;

    bool isArrayType() const override;
    bool isArrayView() const override;

    static ArrayType* Create(Type* elementType);

private:
    Type* mElementType;
};

#endif // BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_ARRAYTYPE_H
