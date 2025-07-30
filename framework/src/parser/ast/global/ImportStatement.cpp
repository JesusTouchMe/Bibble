// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/global/ImportStatement.h"

namespace parser {
    ImportStatement::ImportStatement(symbol::Scope* scope, std::vector<std::string> module, lexer::Token token)
        : ASTNode(scope, nullptr, std::move(token))
        , mModule(std::move(module)) {
        mScope->importedModuleNames[mModule.back()] = constructModuleName();
    }

    const std::vector<std::string>& ImportStatement::getModule() const {
        return mModule;
    }

    void ImportStatement::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
    }

    void ImportStatement::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
    }

    void ImportStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
    }

    bool ImportStatement::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
    }

    std::string ImportStatement::constructModuleName() {
        std::string result;
        result.reserve(mModule.size() * 2);

        for (const auto& name : mModule) {
            result += name;
            result += '/';
        }

        result.pop_back();

        return result;
    }
}
