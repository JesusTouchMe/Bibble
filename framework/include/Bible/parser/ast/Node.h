// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_AST_NODE_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_AST_NODE_H

#include "Bible/codegen/Builder.h"
#include "Bible/codegen/Context.h"

#include "Bible/diagnostic/Diagnostic.h"

#include "Bible/lexer/Token.h"

#include "Bible/symbol/Scope.h"

#include "Bible/type/Type.h"

#include <memory>

namespace parser {
    class ASTNode {
    public:
        ASTNode()  = default;
        ~ASTNode() = default;

        Type* getType() const { return mType; }
        lexer::Token& getDebugToken() { return mPreferredDebugToken; }

        virtual void typeCheck(symbol::Scope* scope, diagnostic::Diagnostics& diag) = 0;
        virtual void emit(codegen::Builder& builder, codegen::Context& ctx, symbol::Scope* scope, diagnostic::Diagnostics& diag) = 0;

    protected:
        Type* mType;
        lexer::Token mPreferredDebugToken;
    };

    using ASTNodePtr = std::unique_ptr<ASTNode>;
}

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_AST_NODE_H
