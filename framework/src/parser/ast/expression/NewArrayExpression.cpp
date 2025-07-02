// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/expression/NewArrayExpression.h"

#include <format>

namespace parser {
    NewArrayExpression::NewArrayExpression(symbol::Scope* scope, ArrayType* arrayType, ASTNodePtr length, lexer::Token token)
        : ASTNode(scope, arrayType, std::move(token))
        , mLength(std::move(length)) {}

    void NewArrayExpression::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        if (statement) return; // i doubt anyone actually cares about the "side effect" of allocating and freeing an array, but i'll change it if someone does

        mLength->codegen(builder, ctx, diag, false);
        builder.createNewArray(mType);
    }

    void NewArrayExpression::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        mLength->semanticCheck(diag, exit, false);
    }

    void NewArrayExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mLength->typeCheck(diag, exit);

        auto longType = Type::Get("long");

        if (mLength->getType() != longType) {
            if (mLength->implicitCast(diag, longType)) {
                mLength = Cast(mLength, longType);
            } else {
                diag.compilerError(mErrorToken.getStartLocation(),
                                   mErrorToken.getEndLocation(),
                                   std::format("no match for '{}new {}{}' with length type '{}{}{}'",
                                               fmt::bold, mType->getName(), fmt::defaults,
                                               fmt::bold, mLength->getType()->getName(), fmt::defaults));
                exit = true;
            }
        }
    }

    bool NewArrayExpression::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}
