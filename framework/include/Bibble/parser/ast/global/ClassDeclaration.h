// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_GLOBAL_CLASSDECLARATION_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_GLOBAL_CLASSDECLARATION_H

#include "Bibble/parser/ast/Node.h"
#include "Bibble/parser/ast/global/Function.h"

#include <moduleweb/class_info.h>

namespace parser {
    enum class ClassModifier : u16 {
        Public = MODULEWEB_CLASS_MODIFIER_PUBLIC,
        Private = MODULEWEB_CLASS_MODIFIER_PRIVATE,
        Abstract = MODULEWEB_CLASS_MODIFIER_ABSTRACT,
        Final = MODULEWEB_CLASS_MODIFIER_FINAL,
    };

    enum class FieldModifier : u16 {
        Public = MODULEWEB_FIELD_MODIFIER_PUBLIC,
        Private = MODULEWEB_FIELD_MODIFIER_PRIVATE,
        Protected = MODULEWEB_FIELD_MODIFIER_PROTECTED,
        Const = MODULEWEB_FIELD_MODIFIER_CONST,
        Volatile = MODULEWEB_FIELD_MODIFIER_VOLATILE,
    };

    struct ClassField {
        ClassField(std::vector<FieldModifier> modifiers, Type* type, std::string name);

        std::vector<FieldModifier> modifiers;
        Type* type;
        std::string name;
    };

    // TODO: implement virtual methods everywhere
    struct ClassMethod {
        ClassMethod(std::vector<FunctionModifier> modifiers, std::string name, FunctionType* type, std::vector<FunctionArgument> arguments, std::vector<ASTNodePtr> body, symbol::ScopePtr scope, lexer::Token errorToken, bool overrides);

        std::vector<FunctionModifier> modifiers; // TODO: method modifiers
        std::string name;
        FunctionType* type;
        std::vector<FunctionArgument> arguments;
        std::vector<ASTNodePtr> body;
        symbol::ScopePtr scope;
        lexer::Token errorToken;

        bool overrides;
    };

    class ClassDeclaration : public ASTNode {
    public:
        ClassDeclaration(std::vector<ClassModifier> modifiers, std::string name, std::vector<ClassField> fields, std::vector<ClassMethod> constructors, std::vector<ClassMethod> methods, symbol::ScopePtr scope, lexer::Token token);

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        std::vector<ClassModifier> mModifiers;
        std::string mName;
        std::vector<ClassField> mFields;
        std::vector<ClassMethod> mConstructors;
        std::vector<ClassMethod> mMethods;
        symbol::ScopePtr mOwnScope;
    };

    using ClassDeclarationPtr = std::unique_ptr<ClassDeclaration>;
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_GLOBAL_CLASSDECLARATION_H
