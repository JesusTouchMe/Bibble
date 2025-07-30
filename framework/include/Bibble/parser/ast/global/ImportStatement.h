// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_GLOBAL_IMPORTSTATEMENT_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_GLOBAL_IMPORTSTATEMENT_H 1

#include "Bibble/parser/ast/Node.h"

namespace parser {
    class ImportStatement : public ASTNode {
    public:
        ImportStatement(symbol::Scope* scope, std::vector<std::string> module, lexer::Token token);

        const std::vector<std::string>& getModule() const;

        void codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) override;

        void semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        std::vector<std::string> mModule;

        std::string constructModuleName();
    };

    using ImportStatementPtr = std::unique_ptr<ImportStatement>;
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_AST_GLOBAL_IMPORTSTATEMENT_H