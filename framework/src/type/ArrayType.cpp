// Copyright 2025 JesusTouchMe

#include "Bibble/type/ArrayType.h"
#include "Bibble/type/ViewType.h"

#include <algorithm>
#include <format>

ArrayType::ArrayType(Type* elementType)
    : Type(std::format("{}[]", elementType->getName()))
    , mElementType(elementType) {}

Type* ArrayType::getElementType() const {
    return mElementType;
}

int ArrayType::getStackSlots() const {
    return 2;
}

JesusASM::Type* ArrayType::getJesusASMType() const {
    return JesusASM::Type::GetArrayType(mElementType->getJesusASMType());
}

codegen::Type ArrayType::getRuntimeType() const {
    return codegen::Type::Category2_Reference;
}

Type::CastLevel ArrayType::castTo(Type* destType) const {
    if (destType->isViewType()) {
        auto viewType = static_cast<ViewType*>(destType);
        if (viewType->getBaseType() == this) {
            return CastLevel::Implicit;
        }
    }

    return CastLevel::Disallowed;
}

bool ArrayType::isArrayType() const {
    return true;
}

bool ArrayType::isArrayView() const {
    return true;
}

static std::vector<std::unique_ptr<ArrayType>> arrayTypes;

ArrayType* ArrayType::Create(Type* elementType) {
    auto it = std::find_if(arrayTypes.begin(), arrayTypes.end(), [elementType](const auto& arrayType) {
        return arrayType->mElementType == elementType;
    });

    if (it != arrayTypes.end()) {
        return it->get();
    }

    arrayTypes.push_back(std::make_unique<ArrayType>(elementType));
    return arrayTypes.back().get();
}
