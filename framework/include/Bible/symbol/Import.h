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
        const std::vector<parser::ASTNodePtr>& importModule(fs::path path, std::string moduleName, diagnostic::Diagnostics& diag);

    private:
        std::vector<fs::path> mModulePaths;
        std::unordered_map<std::string, std::vector<parser::ASTNodePtr>> mImportedModules;
    };
}

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_SYMBOL_IMPORT_H
