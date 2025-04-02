// Copyright 2025 JesusTouchMe

#include "Platform.h"
#include "Subcommands.h"

#include "Bibble/diagnostic/Log.h"

using namespace Bibble;

int main(int argc, char** argv) {
#ifdef PLATFORM_WINDOWS
    SetupWindowsConsole();
#endif

    InitSubcommands();

    if (argc < 2) {
        return CallSubcommand("help");
    }

    std::string subcommand = argv[1];
    std::vector<std::string> args(argv + 2, argv + argc);

    for (const auto& arg : args) {
        if (arg == "-V" || arg == "--verbose") {
            Log::verbose = true;
            break;
        }
    }

    return CallSubcommand(subcommand, args);
}

/*
int main(int argc, char** argv) {
#ifdef PLATFORM_WINDOWS
    SetupWindowsConsole();
#endif

    if (argc < 2) {
        std::cerr << "bibble: no input files\n";
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        fs::path inputPath = arg;
        if (!inputPath.has_extension()) inputPath.replace_extension(".bibble");

        std::ifstream inputFile(inputPath);

        if (!inputFile.is_open()) {
            std::cerr << "bibble: could not find file '" << arg << "'\n";
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
        module->attributes.push_back(std::make_unique<JesusASM::Attribute<std::string_view>>("CompilerID", "Bibble-Lang 1"));

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
 */