// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_SRC_TYPE_INTEGERTYPE_H
#define BIBLE_FRAMEWORK_SRC_TYPE_INTEGERTYPE_H

#include "Bible/type/Type.h"

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

    CastLevel castTo(Type* destType) const override;
    std::string getImplicitCastWarning(Type *destType) const override;

    bool isIntegerType() const override;

private:
    Size mSize;
};

#endif //BIBLE_FRAMEWORK_SRC_TYPE_INTEGERTYPE_H
