// Copyright 2025 JesusTouchMe

#include "Bible/lexer/Lexer.h"

#include "Bible/parser/ImportParser.h"

#include "Bible/symbol/Import.h"

#include <fstream>

namespace symbol {

    ImportManager::ImportManager() {
        addModulePath(".");
    }

    void ImportManager::addModulePath(std::string path) {
        mModulePaths.push_back(std::move(path));
    }

    std::vector<parser::ASTNodePtr> ImportManager::importModule(fs::path path, diagnostic::Diagnostics& diag) {
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
        return nodes;
    }
}