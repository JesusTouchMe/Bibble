// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_SYMBOL_IMPORT_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_SYMBOL_IMPORT_H

#include "Bibble/parser/ast/Node.h"

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
        bool importModule(fs::path path, diagnostic::Diagnostics& diag, Scope* scope);

        void seizeScope(ScopePtr scope);

    private:
        std::vector<fs::path> mModulePaths;
        std::vector<ScopePtr> mScopes;
    };
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_SYMBOL_IMPORT_H
