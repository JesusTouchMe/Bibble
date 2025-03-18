// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_CLASSTYPE_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_CLASSTYPE_H

#include "Bible/type/Type.h"

#include <moduleweb/types.h>

class ClassType : public Type {
public:
    ClassType(std::string_view moduleName, std::string_view name);

    std::string_view getModuleName() const;
    std::string_view getName() const;

    int getStackSlots() const override;
    JesusASM::Type* getJesusASMType() const override;

    CastLevel castTo(Type* destType) const override;

    bool isClassType() const override;

private:
    std::string mModuleName;
    std::string mName;
};

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_CLASSTYPE_H
