// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_FUNCTIONTYPE_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_FUNCTIONTYPE_H

#include "Bibble/type/Type.h"

class FunctionType : public Type {
public:
    FunctionType(Type* returnType, std::vector<Type*> arguments);

    Type* getReturnType() const;
    const std::vector<Type*>& getArgumentTypes() const;

    int getStackSlots() const override;
    JesusASM::Type* getJesusASMType() const override;
    codegen::Type getRuntimeType() const override;

    CastLevel castTo(Type* destType) const override;

    bool isFunctionType() const override;

    static FunctionType* Create(Type* returnType, std::vector<Type*> arguments);

private:
    Type* mReturnType;
    std::vector<Type*> mArguments;
};

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_FUNCTIONTYPE_H
