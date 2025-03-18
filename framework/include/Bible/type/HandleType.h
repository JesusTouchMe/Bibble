// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_HANDLETYPE_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_HANDLETYPE_H

#include "Bible/type/Type.h"

class HandleType : public Type {
public:
    HandleType();

    int getStackSlots() const override;
    JesusASM::Type* getJesusASMType() const override;

    bool isHandleType() const override;
};

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_HANDLETYPE_H
