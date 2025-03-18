// Copyright 2025 JesusTouchMe

#include "Bible/diagnostic/Diagnostic.h"

#include "Bible/type/IntegerType.h"

#include <format>

IntegerType::IntegerType(std::string_view name, Size size)
    : Type(name)
    , mSize(size) {}

IntegerType::Size IntegerType::getSize() const {
    return mSize;
}

int IntegerType::getStackSlots() const {
    if (mSize == Size::Long) {
        return 2;
    } else {
        return 1;
    }
}

JesusASM::Type* IntegerType::getJesusASMType() const {
    return JesusASM::Type::GetBuiltinType(mName);
}

Type::CastLevel IntegerType::castTo(Type* destType) const {
    if (destType->isIntegerType()) {
        auto integerType = static_cast<IntegerType*>(destType);

        if (integerType->mSize < mSize) {
            return Type::CastLevel::ImplicitWarning;
        }

        return Type::CastLevel::Implicit;
    } else if (destType->isBooleanType()) {
        return Type::CastLevel::ImplicitWarning;
    } else if (destType->isCharType()) {
        if (mSize > Size::Short) {
            return Type::CastLevel::ImplicitWarning;
        }

        return Type::CastLevel::Implicit;
    }

    return Type::CastLevel::Disallowed;
}

std::string IntegerType::getImplicitCastWarning(Type* destType) const {
    return std::format("potential loss of data casting '{}{}{}' to '{}{}{}'",
                       fmt::bold, mName, fmt::defaults,
                       fmt::bold, destType->getName(), fmt::defaults);
}

bool IntegerType::isIntegerType() const {
    return true;
}