// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_INTEGERLITERAL_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_INTEGERLITERAL_H

#include "Bibble/parser/ast/Node.h"

#include <JesusASM/tree/instructions/IntInsnNode.h>

#include <cstdint>

namespace parser {
    class IntegerLiteral : public ASTNode {
    public:
        IntegerLiteral(symbol::Scope* scope, std::intmax_t value, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        std::intmax_t mValue;
    };

    using IntegerLiteralPtr = std::unique_ptr<IntegerLiteral>;
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_INTEGERLITERAL_H
