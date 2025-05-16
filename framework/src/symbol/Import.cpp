// Copyright 2025 JesusTouchMe

#include "Bibble/lexer/Lexer.h"

#include "Bibble/parser/Parser.h"

#include "Bibble/symbol/Import.h"

#include <algorithm>
#include <fstream>

namespace symbol {
    ImportManager::ImportManager() {
        //addSearchPath(fs::current_path());
    }

    void ImportManager::addSearchPath(fs::path path) {
        mSearchPaths.push_back(std::move(path));
    }

    SourceFile* ImportManager::importModule(fs::path path, std::string moduleName) {
        //TODO: multimodule

        path.replace_extension(".bibble");

        SourceFile* existing = findExistingSourceFile(path);
        if (existing != nullptr) {
            return existing;
        }

        std::ifstream stream;
        fs::path fullPath;

        for (const auto& searchPath : mSearchPaths) {
            fullPath = searchPath / path;
            stream.open(fullPath);
            if (stream.is_open()) break;
        }

        if (!stream.is_open()) {
            return nullptr;
        }

        std::stringstream buffer;
        buffer << stream.rdbuf();

        std::string text = buffer.str();

        diagnostic::Diagnostics diag;
        diag.setText(text);
        //diag.setImported(true);

        std::string fileName = path.string();
        lexer::Lexer lexer(text, fileName);

        std::vector<lexer::Token> tokens = lexer.lex();

        symbol::ScopePtr scope = std::make_unique<symbol::Scope>(nullptr, std::move(moduleName), true);
        parser::Parser parser(tokens, diag, *this, scope.get());

        auto ast = parser.parse();

        mSourceFiles.emplace_back(std::move(fullPath), std::move(ast), std::move(scope), std::move(diag), std::move(text));

        return &mSourceFiles.back();
    }

    SourceFile* ImportManager::findExistingSourceFile(const fs::path& path) {
        auto it = std::find_if(mSourceFiles.begin(), mSourceFiles.end(), [&path](const SourceFile& file) {
           return file.path == path;
        });

        if (it != mSourceFiles.end()) return &*it;
        return nullptr;
    }
}