// Copyright 2025 JesusTouchMe

#include "Bible/type/ClassType.h"

#include <algorithm>
#include <format>

ClassType::ClassType(std::string_view moduleName, std::string_view name)
    : Type(std::format("{}.{}", moduleName, name))
    , mModuleName(moduleName)
    , mName(name) {}

std::string_view ClassType::getModuleName() const {
    return mModuleName;
}

std::string_view ClassType::getName() const {
    return mName;
}

int ClassType::getStackSlots() const {
    return 2;
}

JesusASM::Type* ClassType::getJesusASMType() const {
    std::string moduleName = mModuleName;
    std::replace(moduleName.begin(), moduleName.end(), '.', '/');

    return JesusASM::Type::GetClassType(moduleName, mName);
}

bool ClassType::isClassType() const {
    return true;
}
