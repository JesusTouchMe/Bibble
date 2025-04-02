// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_AST_EXPRESSION_INTEGERLITERAL_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_AST_EXPRESSION_INTEGERLITERAL_H

#include "Bible/parser/ast/Node.h"

#include <JesusASM/tree/instructions/IntInsnNode.h>

#include <cstdint>

namespace parser {
    class IntegerLiteral : public ASTNode {
    public:
        IntegerLiteral(symbol::Scope* scope, std::intmax_t value, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        std::intmax_t mValue;
    };

    using IntegerLiteralPtr = std::unique_ptr<IntegerLiteral>;
}

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_AST_EXPRESSION_INTEGERLITERAL_H
