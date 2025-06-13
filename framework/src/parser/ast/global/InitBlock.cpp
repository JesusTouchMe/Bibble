// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/global/InitBlock.h"

#include <algorithm>

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
            auto it = std::find_if(ctx.getModule()->functions.begin(), ctx.getModule()->functions.end(), [](const auto& node) {
                return node->name == "#LinkInit";
            });
            if (it == ctx.getModule()->functions.end()) {
                diag.fatalError("'#LinkInit' symbol declared, but no definition was found");
            }

            functionNode = it->get();
        }

        builder.setInsertPoint(&functionNode->instructions);

        for (auto& node : mBody) {
            node->codegen(builder, ctx, diag, true);
        }
    }

    void InitBlock::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        for (auto& node : mBody) {
            node->semanticCheck(diag, exit, true);
        }
    }

    void InitBlock::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        for (auto& node : mBody) {
            node->typeCheck(diag, exit);
        }
    }

    bool InitBlock::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }
}
