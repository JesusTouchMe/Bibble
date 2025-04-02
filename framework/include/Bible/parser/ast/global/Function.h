// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_AST_GLOBAL_FUNCTION_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_AST_GLOBAL_FUNCTION_H

#include "Bible/parser/ast/Node.h"

#include <moduleweb/function_info.h>

namespace parser {
    struct FunctionArgument {
        Type* type;
        std::string name;
    };

    enum class FunctionModifier : u16 {
        Public = MODULEWEB_FUNCTION_MODIFIER_PUBLIC,
        Private = MODULEWEB_FUNCTION_MODIFIER_PRIVATE,
        Pure = MODULEWEB_FUNCTION_MODIFIER_PURE,
        Async = MODULEWEB_FUNCTION_MODIFIER_ASYNC,
        Native = MODULEWEB_FUNCTION_MODIFIER_NATIVE,
    };

    class Function : public ASTNode {
    public:
        Function(std::vector<FunctionModifier> modifiers, std::string name, FunctionType* type, std::vector<FunctionArgument> arguments, std::vector<ASTNodePtr> body, symbol::ScopePtr scope, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        std::vector<FunctionModifier> mModifiers;
        std::string mName;
        std::vector<FunctionArgument> mArguments;
        std::vector<ASTNodePtr> mBody;
        symbol::ScopePtr mOwnScope;
    };

    using FunctionPtr = std::unique_ptr<Function>;
}

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_AST_GLOBAL_FUNCTION_H
