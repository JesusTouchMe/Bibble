// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_CLASSTYPE_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_CLASSTYPE_H

#include "Bible/lexer/SourceLocation.h"

#include "Bible/type/Type.h"

#include <moduleweb/types.h>

class ClassType : public Type {
public:
    ClassType(std::string_view moduleName, std::string_view name);

    std::string_view getModuleName() const;
    std::string_view getName() const;

    int getStackSlots() const override;
    JesusASM::Type* getJesusASMType() const override;
    codegen::Type getRuntimeType() const override;

    CastLevel castTo(Type* destType) const override;

    void resolve(symbol::Scope* scope, diagnostic::Diagnostics& diag) override;

    bool isClassType() const override;

    static ClassType* Create(std::string_view moduleName, std::string_view name);

private:
    std::string mModuleName;
    std::string mName;
};

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_CLASSTYPE_H
