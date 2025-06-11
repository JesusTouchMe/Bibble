// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/global/InitBlock.h"

namespace parser {
    InitBlock::InitBlock(symbol::ScopePtr scope, std::vector<ASTNodePtr> body, lexer::Token token)
        : ASTNode(scope->parent, FunctionType::Create(Type::Get("void"), {}), std::move(token))
        , mBody(std::move(body))
        , mOwnScope(std::move(scope)) {}

    void InitBlock::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        auto functionType = static_cast<FunctionType*>(mType);
        auto function = mScope->findFunction("#LinkInit", functionType);
        JesusASM::tree::FunctionNode* functionNode;

        if (function == nullptr) {
            function = mScope->createFunction("#LinkInit", functionType, 0);
            functionNode = builder.addFunction(0, "#LinkInit", functionType->getJesusASMType());
        } else {
            auto it = std::find
        }
    }

    void InitBlock::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
    }

    void InitBlock::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
    }

    bool InitBlock::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
    }
}
