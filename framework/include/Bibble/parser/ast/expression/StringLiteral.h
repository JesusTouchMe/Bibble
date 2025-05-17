// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_STRINGLITERAL_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_STRINGLITERAL_H

#include "Bibble/parser/ast/Node.h"

namespace parser {
    class StringLiteral : public ASTNode {
    public:
        StringLiteral(symbol::Scope* scope, std::string value, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        std::string mValue;
    };

    using StringLiteralPtr = std::unique_ptr<StringLiteral>;
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_STRINGLITERAL_H
