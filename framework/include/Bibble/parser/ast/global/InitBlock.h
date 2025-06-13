// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_GLOBAL_INITBLOCK_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_GLOBAL_INITBLOCK_H 1

#include "Bibble/parser/ast/Node.h"

namespace parser {
    class InitBlock : public ASTNode {
    public:
        InitBlock(symbol::ScopePtr scope, std::vector<ASTNodePtr> body, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        std::vector<ASTNodePtr> mBody;
        symbol::ScopePtr mOwnScope;
    };

    using InitBlockPtr = std::unique_ptr<InitBlock>;
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_GLOBAL_INITBLOCK_H