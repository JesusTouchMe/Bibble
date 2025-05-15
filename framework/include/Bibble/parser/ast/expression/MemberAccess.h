// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_MEMBERACCESS_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_MEMBERACCESS_H 1

#include "Bibble/parser/ast/Node.h"

namespace parser {
    class MemberAccess : public ASTNode {
    friend class CallExpression;
    public:
        MemberAccess(symbol::Scope* scope, ASTNodePtr classNode, std::string id, lexer::Token operatorToken, lexer::Token token);

        ASTNode* getClass() const;
        std::string_view getId() const;
        symbol::ClassSymbol* getClassSymbol() const;
        ClassType* getClassType() const;

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        lexer::Token mOperatorToken;

        ASTNodePtr mClass;
        std::string mId;

        symbol::ClassSymbol* mClassSymbol;
        ClassType* mClassType;
    };

    using MemberAccessPtr = std::unique_ptr<MemberAccess>;
}

#endif // BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_EXPRESSION_MEMBERACCESS_H
