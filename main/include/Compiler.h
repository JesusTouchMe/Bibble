// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_COMPILER_H
#define BIBBLE_COMPILER_H 1

#include "Bibble/codegen/Builder.h"
#include "Bibble/codegen/Context.h"

#include "Bibble/diagnostic/Diagnostic.h"

#include "Bibble/lexer/Token.h"

#include "Bibble/parser/ast/Node.h"

#include <toml.hpp>

#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

struct Module {
    fs::path path;
    std::string pathString;
    std::string moduleName;
    std::string text;

    std::vector<lexer::Token> tokens;
    std::vector<parser::ASTNodePtr> ast;

    symbol::ScopePtr scope;
};

class Compiler {
public:
    explicit Compiler(diagnostic::Diagnostics& diag);

    void build();

private:
    toml::table mConfig;

    diagnostic::Diagnostics& mDiag;

    std::unordered_map<std::string, std::vector<fs::path>> mMultiModules;
    std::unordered_map<fs::path, Module> mModules;
    std::vector<fs::path> mCompiledModules;

    void parseConfig(fs::path configFilePath);

    fs::path getSourceDir();
    fs::path getBuildDir();

    void compileModules(fs::path projectDir);

    void lexOne(fs::path inputFilePath);
    void parseModuleName(fs::path inputFilePath, fs::path inputFilePathRelative);
    void parseOne(fs::path inputFilePath);
    void doImports(fs::path inputFilePath);
    void compileModule(fs::path inputFilePath, fs::path outputFilePath);

    void addModuleBloat(codegen::ModuleNode* module);
};

#endif // BIBBLE_COMPILER_H
