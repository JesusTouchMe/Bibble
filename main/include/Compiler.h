// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_MAIN_INCLUDE_COMPILER_H
#define BIBBLE_MAIN_INCLUDE_COMPILER_H

#include "Bibble/symbol/Import.h"

#include <JesusASM/tree/ModuleNode.h>

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace Bibble {
    class Compiler {
    public:
        void setModuleName(std::string moduleName);

        void setInput(fs::path input);
        void setOutput(fs::path output);

        void addImportPath(fs::path path);

        void compile();

    private:
        std::string mModuleName;
        fs::path mInput;
        fs::path mOutput;

        symbol::ImportManager mImportManager;

        void addModuleBloat(JesusASM::tree::ModuleNode* module);
    };
}

#endif //BIBBLE_MAIN_INCLUDE_COMPILER_H
