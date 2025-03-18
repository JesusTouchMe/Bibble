// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_CHARTYPE_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_CHARTYPE_H

#include "Bible/type/Type.h"

class CharType : public Type {
public:
    CharType();

    int getStackSlots() const override;
    JesusASM::Type* getJesusASMType() const override;

    bool isCharType() const override;
};

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_CHARTYPE_H
