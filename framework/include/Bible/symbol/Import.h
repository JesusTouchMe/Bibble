// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_SYMBOL_IMPORT_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_SYMBOL_IMPORT_H

#include "Bible/parser/ast/Node.h"

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace symbol {
    class ImportManager {
    public:
        ImportManager();

        void addModulePath(std::string path);
        std::vector<parser::ASTNodePtr> importModule(fs::path path, diagnostic::Diagnostics& diag);

    private:
        std::vector<std::string> mModulePaths;
    };
}

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_SYMBOL_IMPORT_H
