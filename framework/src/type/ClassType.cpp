// Copyright 2025 JesusTouchMe

#include "Bibble/symbol/Scope.h"

#include "Bibble/type/ClassType.h"

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

codegen::Type ClassType::getRuntimeType() const {
    return codegen::Type::Category2_Reference;
}

Type::CastLevel ClassType::castTo(Type* destType) const {
    //TODO: Explicit casting to handle by opening a handle to the object in the VM, making it manually managed
    //TODO: Implicit casting to base class
    return CastLevel::Disallowed;
}

void ClassType::resolve(symbol::Scope* scope, diagnostic::Diagnostics& diag) {
    auto classSymbol = scope->findClass({ mModuleName, mName }); // TODO: does this work?
    if (classSymbol == nullptr) {
        diag.fatalError(std::format("cannot resolve class '{}{}::{}{}'",
                                    fmt::bold, mModuleName, mName, fmt::defaults));
    }
}

bool ClassType::isClassType() const {
    return true;
}

std::vector<std::unique_ptr<ClassType>> classTypes;

ClassType* ClassType::Create(std::string_view moduleName, std::string_view name) {
    auto it = std::find_if(classTypes.begin(), classTypes.end(), [moduleName, name](const auto& type) {
        return type->mModuleName == moduleName && type->mName == name;
    });

    if (it != classTypes.end()) {
        return it->get();
    }

    classTypes.push_back(std::make_unique<ClassType>(moduleName, name));
    return classTypes.back().get();
}

