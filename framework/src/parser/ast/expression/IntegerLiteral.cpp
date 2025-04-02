// Copyright 2025 JesusTouchMe

#include "Bible/parser/ast/expression/Integerliteral.h"

#include "Bible/type/IntegerType.h"

#include <format>
#include <limits>

namespace parser {
    IntegerLiteral::IntegerLiteral(symbol::Scope* scope, std::intmax_t value, lexer::Token token)
        : ASTNode(scope, Type::Get("int"), std::move(token))
        , mValue(value) {}

    void IntegerLiteral::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) {
        builder.createLdc(mType, mValue);
    }

    void IntegerLiteral::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {

    }

    void IntegerLiteral::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {

    }

    bool IntegerLiteral::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        if (destType->isIntegerType()) {
            switch (static_cast<IntegerType*>(destType)->getSize()) {
                case IntegerType::Size::Byte:
                    if (mValue < std::numeric_limits<int8_t>::min() || mValue > std::numeric_limits<int8_t>::max()) {
                        auto narrowed = static_cast<int8_t>(mValue);
                        diag.compilerWarning("implicit",
                                             mErrorToken.getStartLocation(),
                                             mErrorToken.getEndLocation(),
                                             std::format(
                                                     "integer literal with value '{}{}{}' is being narrowed to '{}{}{}'",
                                                     fmt::bold, mValue, fmt::defaults,
                                                     fmt::bold, (int) narrowed, fmt::defaults));
                    }

                    break;
                case IntegerType::Size::Short:
                    if (mValue < std::numeric_limits<int16_t>::min() || mValue > std::numeric_limits<int16_t>::max()) {
                        auto narrowed = static_cast<int16_t>(mValue);
                        diag.compilerWarning("implicit",
                                             mErrorToken.getStartLocation(),
                                             mErrorToken.getEndLocation(),
                                             std::format(
                                                     "integer literal with value '{}{}{}' is being narrowed to '{}{}{}'",
                                                     fmt::bold, mValue, fmt::defaults,
                                                     fmt::bold, narrowed, fmt::defaults));
                    }

                    break;
                case IntegerType::Size::Int:
                    if (mValue < std::numeric_limits<int32_t>::min() || mValue > std::numeric_limits<int32_t>::max()) {
                        auto narrowed = static_cast<int32_t>(mValue);
                        diag.compilerWarning("implicit",
                                             mErrorToken.getStartLocation(),
                                             mErrorToken.getEndLocation(),
                                             std::format(
                                                     "integer literal with value '{}{}{}' is being narrowed to '{}{}{}'",
                                                     fmt::bold, mValue, fmt::defaults,
                                                     fmt::bold, narrowed, fmt::defaults));
                    }

                    break;
                case IntegerType::Size::Long:
                    if (mValue < std::numeric_limits<int64_t>::min() || mValue > std::numeric_limits<int64_t>::max()) {
                        auto narrowed = static_cast<int64_t>(mValue);
                        diag.compilerWarning("implicit",
                                             mErrorToken.getStartLocation(),
                                             mErrorToken.getEndLocation(),
                                             std::format(
                                                     "integer literal with value '{}{}{}' is being narrowed to '{}{}{}'",
                                                     fmt::bold, mValue, fmt::defaults,
                                                     fmt::bold, narrowed, fmt::defaults));
                    }

                    break;
            }

            mType = destType;
            return true;
        }

        return false;
    }
}