// Copyright 2025 JesusTouchMe

// this might need some explaining:
// a view type is essentially a type that represents a view into another reference type and is resolved at compile type
// this results in no runtime checks and no clones to simply provide a view of an object
// this obviously comes with the downside of asm being able to mutate a returned value, but if you're doing that you don't deserve to use jesusasm

#ifndef BIBBLE_FRAMEWORK_SRC_TYPE_VIEWTYPE_H
#define BIBBLE_FRAMEWORK_SRC_TYPE_VIEWTYPE_H 1

#include "Bibble/type/Type.h"

class ViewType : public Type {
public:
    ViewType(Type* base);

    Type* getBaseType() const;

    JesusASM::Type* getJesusASMType() const override;
    codegen::Type getRuntimeType() const override;

    CastLevel castTo(Type* destType) const override;

    bool isClassView() const override;
    bool isArrayView() const override;
    bool isViewType() const override;

    static ViewType* Create(Type* base);

private:
    Type* mBaseType;
};

#endif // BIBBLE_FRAMEWORK_SRC_TYPE_VIEWTYPE_H

