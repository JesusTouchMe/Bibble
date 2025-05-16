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
    struct SourceFile {
        fs::path path;
        std::vector<parser::ASTNodePtr> ast;
        ScopePtr scope;
        diagnostic::Diagnostics diag;
        std::string sourceCode;
    };

    class ImportManager {
    public:
        ImportManager();

        void addSearchPath(fs::path path);
        SourceFile* importModule(fs::path path, std::string moduleName);

        SourceFile* findExistingSourceFile(const fs::path& path);

    private:
        std::vector<fs::path> mSearchPaths;
        std::vector<SourceFile> mSourceFiles;
    };
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_SYMBOL_IMPORT_H
