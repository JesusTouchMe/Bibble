// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_SRC_TYPE_INTEGERTYPE_H
#define BIBLE_FRAMEWORK_SRC_TYPE_INTEGERTYPE_H

#include "Bible/type/Type.h"

class IntegerType : public Type {
public:
    IntegerType(std::string_view name, int stackSlots);

    int getStackSlots() const override;
    JesusASM::Type* getJesusASMType() const override;

    bool isIntegerType() const override;

private:
    int mStackSlots;
};

#endif //BIBLE_FRAMEWORK_SRC_TYPE_INTEGERTYPE_H
