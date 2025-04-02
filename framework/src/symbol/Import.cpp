// Copyright 2025 JesusTouchMe

#include "Bible/lexer/Lexer.h"

#include "Bible/parser/ImportParser.h"

#include "Bible/symbol/Import.h"

#include <algorithm>
#include <fstream>

namespace symbol {
    ImportManager::ImportManager() {
        addModulePath(fs::current_path());
    }

    void ImportManager::addModulePath(fs::path path) {
        mModulePaths.push_back(std::move(path));
    }

    std::vector<parser::ASTNodePtr> ImportManager::importModule(fs::path path, diagnostic::Diagnostics& diag, Scope* scope) {
        path += ".bible";

        std::ifstream stream;

        for (const auto& modulePath : mModulePaths) {
            stream.open(modulePath / path);
            if (stream.is_open()) break;
        }

        std::stringstream buffer;
        buffer << stream.rdbuf();

        std::string text = buffer.str();

        diagnostic::Diagnostics importDiag;
        importDiag.setText(buffer.str());
        importDiag.setImported(true);

        std::string fileName = path.string();
        lexer::Lexer lexer(text, fileName);

        std::vector<lexer::Token> tokens = lexer.lex();

        parser::ImportParser parser(tokens, importDiag, *this, scope);

        std::vector<parser::ASTNodePtr> nodes = parser.parse();
        return nodes;
    }

    void ImportManager::seizeScope(ScopePtr scope) {
        mScopes.push_back(std::move(scope));
    }
}