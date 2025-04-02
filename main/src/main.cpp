// Copyright 2025 JesusTouchMe

#include "Bible/codegen/Builder.h"

#include "Bible/diagnostic/Diagnostic.h"

#include "Bible/lexer/Lexer.h"
#include "Bible/lexer/Token.h"

#include "Bible/parser/Parser.h"

#include "Bible/symbol/Import.h"

#include "Bible/type/Type.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

#include <filesystem>
#include <format>
#include <fstream>

namespace fs = std::filesystem;

#ifdef PLATFORM_WINDOWS
void SetupWindowsConsole() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        std::cout << "failed to find stdout (windows specific issue)\n";
        std::cout << "console output will have weird formatting\n";
        return;
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
        std::cout << "failed to get stdout console mode (windows specific issue)\n";
        std::cout << "console output will have weird formatting\n";
        return;
    }

    dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
    if (hErr == INVALID_HANDLE_VALUE) {
        std::cout << "failed to find stderr (windows specific issue)\n";
        std::cout << "console output will have weird formatting\n";
        return;
    }

    if (!GetConsoleMode(hErr, &dwMode)) {
        std::cout << "failed to get stderr console mode (windows specific issue)\n";
        std::cout << "console output will have weird formatting\n";
        return;
    }

    dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hErr, dwMode);
}
#endif

int main(int argc, char** argv) {
#ifdef PLATFORM_WINDOWS
    SetupWindowsConsole();
#endif

    if (argc < 2) {
        std::cerr << "bible: no input files\n";
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        std::ifstream inputFile(arg);

        if (!inputFile.is_open()) {
            std::cerr << "bible: could not find file '" << arg << "'\n";
            return 1;
        }

        fs::path fullInputFilePath = fs::current_path() / arg;
        std::string fullInputPathName = fullInputFilePath.string();

        std::stringstream buffer;
        buffer << inputFile.rdbuf();
        std::string text = buffer.str();

        diagnostic::Diagnostics diag;
        diag.setText(text);

        Type::Init();

        lexer::Lexer lexer(text, fullInputPathName);
        auto tokens = lexer.lex();

        symbol::ImportManager importManager;
        symbol::ScopePtr globalScope = std::make_unique<symbol::Scope>(nullptr, "", true);
        parser::Parser parser(tokens, diag, importManager, globalScope.get());

        auto ast = parser.parse();

        bool hadErrors = false;
        for (auto& node : ast) {
            node->typeCheck(diag, hadErrors);
        }
        if (hadErrors) return 1;

        hadErrors = false;
        for (auto& node : ast) {
            node->semanticCheck(diag, hadErrors, true);
        }
        if (hadErrors) return 1;

        std::string moduleName = arg;
        std::replace(moduleName.begin(), moduleName.end(), '\\', '.');
        std::replace(moduleName.begin(), moduleName.end(), '/', '.');

        moduleName = moduleName.substr(0, moduleName.find_last_of('.'));

        auto module = std::make_unique<JesusASM::tree::ModuleNode>(1, std::move(moduleName));

        // bloat up the module a lil
        module->attributes.push_back(std::make_unique<JesusASM::Attribute<std::string_view>>("CompilerID", "Bible-Lang 1"));

        codegen::Context ctx(std::move(module));
        codegen::Builder builder(ctx);

        for (auto& node : ast) {
            node->codegen(builder, ctx, diag);
        }

        moduleweb::ModuleBuilder moduleBuilder;
        moduleweb::ModuleInfo moduleInfo;

        ctx.getModule()->emit(moduleBuilder, moduleInfo);

        moduleInfo.print();

        fs::path outFile = arg;
        outFile.replace_extension(".jmod");

        std::string outFileName = outFile.string();
        moduleweb::FileOutStream out(outFileName);

        moduleInfo.emit(out);
    }

    return 0;
}