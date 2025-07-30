// Copyright 2025 JesusTouchMe

#include "Bibble/type/ArrayType.h"
#include "Bibble/type/ViewType.h"

#include <algorithm>
#include <format>

#include "Bibble/type/ClassType.h"

ArrayType::ArrayType(Type* elementType)
    : Type(std::format("{}[]", elementType->getName()))
    , mElementType(elementType) {}

Type* ArrayType::getElementType() const {
    return mElementType;
}

JesusASM::Type* ArrayType::getJesusASMType() const {
    return JesusASM::Type::GetArrayType(mElementType->getJesusASMType());
}

codegen::Type ArrayType::getRuntimeType() const {
    return codegen::Type::Reference;
}

Type::CastLevel ArrayType::castTo(Type* destType) const {
    if (destType == this) return CastLevel::Implicit;

    if (destType->isClassView()) {
        bool objectType;
        if (destType->isViewType()) {
            auto type = static_cast<ViewType*>(destType)->getBaseType();
            objectType = type == Type::Get("object");
        } else {
            auto type = static_cast<ClassType*>(destType);
            objectType = type == Type::Get("object");
        }

        if (objectType) {
            return CastLevel::Implicit;
        }
    }

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
