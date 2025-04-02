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
        using ASTNodePtr = std::unique_ptr<ASTNode>;

        ASTNode(symbol::Scope* scope, lexer::Token errorToken) : mScope(scope), mType(nullptr), mErrorToken(std::move(errorToken)) {}
        ASTNode(symbol::Scope* scope, Type* type, lexer::Token errorToken) : mScope(scope), mType(type), mErrorToken(std::move(errorToken)) {}
        virtual ~ASTNode() = default;

        symbol::Scope* getScope() const { return mScope; }
        Type* getType() const { return mType; }
        const lexer::Token& getErrorToken() const { return mErrorToken; }

        virtual void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) = 0;

        virtual void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) = 0;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) = 0;
        virtual bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) = 0;
        bool implicitCast(diagnostic::Diagnostics& diag, Type* destType);

        static ASTNodePtr Cast(ASTNodePtr& node, Type* destType);

    protected:
        symbol::Scope* mScope;
        Type* mType;
        lexer::Token mErrorToken;
    };

    using ASTNodePtr = std::unique_ptr<ASTNode>;
}

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_AST_NODE_H
