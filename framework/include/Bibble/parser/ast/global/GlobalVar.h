// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_GLOBAL_VAR_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_GLOBAL_VAR_H 1

#include "Bibble/parser/ast/Node.h"

#include <moduleweb/global_var_info.h>

namespace parser {
    enum class GlobalVarModifier : u16 {
        Public = MODULEWEB_GLOBAL_VAR_MODIFIER_PUBLIC,
        Private = MODULEWEB_GLOBAL_VAR_MODIFIER_PRIVATE,
        Volatile = MODULEWEB_GLOBAL_VAR_MODIFIER_VOLATILE,
    };

    class GlobalVar : public ASTNode {
    public:
        GlobalVar(std::vector<GlobalVarModifier> modifiers, symbol::Scope* scope, Type* type, std::string name, ASTNodePtr initialValue, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        std::vector<GlobalVarModifier> mModifiers;
        std::string mName;
        ASTNodePtr mInitialValue;
    };

    using GlobalVarPtr = std::unique_ptr<GlobalVar>;
}

#endif // BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_GLOBAL_VAR_H
