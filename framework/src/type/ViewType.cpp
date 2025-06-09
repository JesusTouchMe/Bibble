// Copyright 2025 JesusTouchMe

#include "Bibble/type/ViewType.h"

#include <algorithm>
#include <format>

ViewType::ViewType(Type* base)
    : Type(std::format("view<{}>", base->getName()))
      , mBaseType(base) {
}

Type* ViewType::getBaseType() const {
    return mBaseType;
}

int ViewType::getStackSlots() const {
    return mBaseType->getStackSlots();
}

JesusASM::Type* ViewType::getJesusASMType() const {
    return mBaseType->getJesusASMType();
}

codegen::Type ViewType::getRuntimeType() const {
    return mBaseType->getRuntimeType();
}

Type::CastLevel ViewType::castTo(Type* destType) const {
    if (destType->isViewType()) {
        auto viewType = static_cast<ViewType*>(destType);
        return mBaseType->castTo(viewType->getBaseType());
    }

    return CastLevel::Disallowed;
}

bool ViewType::isClassView() const {
    return mBaseType->isClassType();
}

bool ViewType::isArrayView() const {
    return mBaseType->isArrayType();
}

bool ViewType::isViewType() const {
    return true;
}

std::vector<std::unique_ptr<ViewType>> viewTypes;

ViewType* ViewType::Create(Type* base) {
    auto it = std::find_if(viewTypes.begin(), viewTypes.end(), [base](const auto& type) {
        return type->getBaseType() == base;
    });

    if (it != viewTypes.end()) {
        return it->get();
    }

    viewTypes.push_back(std::make_unique<ViewType>(base));
    return viewTypes.back().get();
}
