// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_AST_GLOBAL_FUNCTION_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_AST_GLOBAL_FUNCTION_H

#include "Bible/parser/ast/Node.h"

namespace parser {
    struct FunctionArgument {
        std::string name;
        Type* type;
    };

    class Function : public ASTNode {
    public:
        Function(FunctionType* type, std::vector<FunctionArgument> arguments, std::string_view name, std::vector<ASTNodePtr>&& body, symbol::Scope* scope);

        Type* getReturnType() const;

        void typeCheck(symbol::Scope* scope, diagnostic::Diagnostics& diag) override;
        void emit(codegen::Builder& builder, codegen::Context& ctx, symbol::Scope* scope, diagnostic::Diagnostics& diag) override;

    private:
        FunctionType* mFunctionType;
        std::vector<FunctionArgument> mArguments;
        std::string name;
        std::vector<ASTNodePtr> mBody;
        symbol::ScopePtr mScope;
    };
}

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_AST_GLOBAL_FUNCTION_H
