// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_BOOLEANLITERAL_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_BOOLEANLITERAL_H

#include "Bibble/parser/ast/Node.h"

namespace parser {
    class BooleanLiteral : public ASTNode {
    public:
        BooleanLiteral(symbol::Scope* scope, bool value, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        bool mValue;
    };

    using BooleanLiteralPtr = std::unique_ptr<BooleanLiteral>;
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_BOOLEANLITERAL_H
