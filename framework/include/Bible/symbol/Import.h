// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_SYMBOL_IMPORT_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_SYMBOL_IMPORT_H

#include "Bible/parser/ast/Node.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

namespace symbol {
    class ImportManager {
    public:
        ImportManager();

        void addModulePath(fs::path path);
        std::vector<parser::ASTNodePtr> importModule(fs::path path, diagnostic::Diagnostics& diag, Scope* scope);

        void seizeScope(ScopePtr scope);

    private:
        std::vector<fs::path> mModulePaths;
        std::vector<ScopePtr> mScopes;
    };
}

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_SYMBOL_IMPORT_H
