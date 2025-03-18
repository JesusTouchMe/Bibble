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

    const std::vector<parser::ASTNodePtr>& ImportManager::importModule(fs::path path, std::string moduleName, diagnostic::Diagnostics& diag) {
        auto it = std::find_if(mImportedModules.begin(), mImportedModules.end(), [&moduleName](const auto& module) {
            return module.first == moduleName;
        });

        if (it != mImportedModules.end()) {
            return it->second;
        }

        path += ".bible";

        std::ifstream stream;

        for (const auto& modulePath : mModulePaths) {
            stream.open(modulePath / path);
            if (stream.is_open()) break;
        }

        std::stringstream buffer;
        buffer << stream.rdbuf();

        diagnostic::Diagnostics importDiag;
        importDiag.setErrorSender("bible");
        importDiag.setFileName(path.string());
        importDiag.setText(buffer.str());
        importDiag.setImported(true);

        std::string fileName = path.string();
        lexer::Lexer lexer(buffer.str(), fileName);

        std::vector<lexer::Token> tokens = lexer.lex();

        parser::ImportParser parser(tokens, importDiag, *this);

        std::vector<parser::ASTNodePtr> nodes = parser.parse();
        auto [it1, _] = mImportedModules.emplace(std::move(moduleName), std::move(nodes));
        return it1->second;
    }
}