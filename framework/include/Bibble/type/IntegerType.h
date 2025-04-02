// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_SRC_TYPE_INTEGERTYPE_H
#define BIBBLE_FRAMEWORK_SRC_TYPE_INTEGERTYPE_H

#include "Bibble/type/Type.h"

class IntegerType : public Type {
public:
    enum class Size {
        Byte = 1,
        Short = 2,
        Int = 4,
        Long = 8
    };

    IntegerType(std::string_view name, Size size);

    Size getSize() const;

    int getStackSlots() const override;
    JesusASM::Type* getJesusASMType() const override;
    codegen::Type getRuntimeType() const override;

    CastLevel castTo(Type* destType) const override;
    std::string getImplicitCastWarning(Type *destType) const override;

    bool isIntegerType() const override;

private:
    Size mSize;
};

#endif //BIBBLE_FRAMEWORK_SRC_TYPE_INTEGERTYPE_H
