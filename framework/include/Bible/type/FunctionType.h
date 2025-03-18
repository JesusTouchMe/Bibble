// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_FUNCTIONTYPE_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_FUNCTIONTYPE_H

#include "Bible/type/Type.h"

class FunctionType : public Type {
public:
    FunctionType(Type* returnType, std::vector<Type*> arguments);

    Type* getReturnType() const;
    const std::vector<Type*>& getArgumentTypes() const;

    int getStackSlots() const override;
    JesusASM::Type* getJesusASMType() const override;

    bool isFunctionType() const override;

    static FunctionType* Create(Type* returnType, std::vector<Type*> arguments);

private:
    Type* mReturnType;
    std::vector<Type*> mArguments;
};

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_FUNCTIONTYPE_H
