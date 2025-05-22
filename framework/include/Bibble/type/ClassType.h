// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_CLASSTYPE_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_CLASSTYPE_H

#include "Bibble/lexer/SourceLocation.h"

#include "Bibble/type/Type.h"

#include <moduleweb/types.h>

class ClassType : public Type {
public:
    ClassType(std::string_view moduleName, std::string_view name, ClassType* baseType);

    std::string_view getModuleName() const;
    std::string_view getName() const;
    ClassType* getBaseType() const;

    int getStackSlots() const override;
    JesusASM::Type* getJesusASMType() const override;
    codegen::Type getRuntimeType() const override;

    CastLevel castTo(Type* destType) const override;

    void resolve(symbol::Scope* scope, diagnostic::Diagnostics& diag) override;

    bool isClassType() const override;

    static ClassType* Find(std::string_view moduleName, std::string_view name);
    static ClassType* Create(std::string_view moduleName, std::string_view name, ClassType* baseType);

private:
    std::string mModuleName;
    std::string mName;
    ClassType* mBaseType;
};

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_CLASSTYPE_H
