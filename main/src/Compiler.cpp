// Copyright 2025 JesusTouchMe

#include "Compiler.h"
#include "HeaderBuilder.h"

#include "Bibble/codegen/pass/optimizer/DeadCodeElimination.h"

#include "Bibble/codegen/pass/CommonPass.h"

#include "Bibble/diagnostic/Log.h"

#include "Bibble/lexer/Lexer.h"

#include "Bibble/parser/Parser.h"

#include <algorithm>
#include <format>
#include <sstream>
#include <set>

Compiler::Compiler(diagnostic::Diagnostics& diag)
    : mDiag(diag) {}

void Compiler::build() {
    fs::path projectDir = std::filesystem::current_path();

    if (!fs::is_directory(projectDir)) {
        mDiag.fatalError(std::format("'{}{}{}' is not a directory", fmt::bold, projectDir.string(), fmt::defaults));
    }

    fs::path config = projectDir / "bibbler.toml";
    if (!fs::exists(config)) {
        mDiag.fatalError(std::format("'{}{}{}' is missing '{}bibbler.toml{}'", fmt::bold, projectDir.string(), fmt::defaults, fmt::bold, fmt::defaults));
    }

    parseConfig(config);

    Type::Init();

    compileModules(projectDir);
}

void Compiler::parseConfig(fs::path configFilePath) {
    try {
        mConfig = toml::parse_file(configFilePath.string());
    } catch (const toml::parse_error& error) {
        mDiag.fatalError(std::format("error parsing project configuration: {}", error.description()));
    }
}

fs::path Compiler::getSourceDir() {
    if (!mConfig.contains("build")) return "<error-source-directory>";
    auto build = mConfig["build"].as_table();
    if (build == nullptr) return "<error-source-directory>";

    auto output = build->at("SourceDir").as_string();
    if (output == nullptr) return "<error-source-directory>";

    return output->get();
}

fs::path Compiler::getBuildDir() {
    if (!mConfig.contains("build")) return "<error-build-directory>";
    auto build = mConfig["build"].as_table();
    if (build == nullptr) return "<error-build-directory>";

    auto output = build->at("BuildDir").as_string();
    if (output == nullptr) return "<error-build-directory>";

    return output->get();
}

void Compiler::compileModules(fs::path projectDir) {
    fs::path sourceDir = projectDir / getSourceDir();
    fs::path sourceDirRelative = getSourceDir();
    fs::path buildDir = projectDir / getBuildDir();
    std::vector<std::pair<fs::path, fs::path>> files;

    for (fs::recursive_directory_iterator it(sourceDir); it != fs::end(it); ++it) {
        if (it->is_regular_file() && it->path().extension() == ".bibble") {
            //TODO: if file wasn't modified since last compile, skip

            fs::path inputFile = it->path();
            fs::path inputFileRelative = fs::relative(inputFile, sourceDir);
            fs::path outputFile = buildDir / sourceDirRelative / inputFileRelative;
            outputFile.replace_extension(".jmod");

            fs::create_directory(outputFile.parent_path());

            mCompiledModules.push_back(outputFile);
            files.emplace_back(inputFile, outputFile);

            lexOne(inputFile);
            parseModuleName(inputFile, inputFileRelative);
            parseOne(inputFile);
        }
    }

    for (auto& file : files) {
        doImports(file.first);
    }

    for (auto& file : files) {
        compileModule(file.first, file.second);
    }
}

void Compiler::lexOne(fs::path inputFilePath) {
    std::ifstream inputFile(inputFilePath);
    std::stringstream buffer;
    buffer << inputFile.rdbuf();

    mModules[inputFilePath].pathString = inputFilePath.string();
    mModules[inputFilePath].text = std::move(buffer).str();

    inputFile.close();

    lexer::Lexer lexer(mModules[inputFilePath].text, mModules[inputFilePath].pathString);
    mModules[inputFilePath].tokens = lexer.lex();

    mDiag.addText(mModules[inputFilePath].pathString, mModules[inputFilePath].text);
}

void Compiler::parseModuleName(fs::path inputFilePath, fs::path inputFilePathRelative) {
    std::string moduleName = inputFilePathRelative.string();
    std::ranges::replace(moduleName, '\\', '/');
    moduleName = moduleName.substr(0, moduleName.find_last_of('.'));

    mMultiModules[moduleName].push_back(inputFilePath);
    mModules[inputFilePath].moduleName = std::move(moduleName);
}

void Compiler::parseOne(fs::path inputFilePath) {
    mModules[inputFilePath].scope = std::make_unique<symbol::Scope>(nullptr, mModules[inputFilePath].moduleName, true);

    std::string moduleName = mModules[inputFilePath].moduleName;
    std::string shortModuleName;
    auto pos = moduleName.find_last_of('/');

    if (pos == std::string::npos) {
        shortModuleName = moduleName;
    } else {
        shortModuleName = moduleName.substr(pos + 1);
    }
    mModules[inputFilePath].scope->importedModuleNames[shortModuleName] = std::move(moduleName);

    auto& tokens = mModules[inputFilePath].tokens;

    parser::Parser parser(tokens, mDiag, mModules[inputFilePath].scope.get(), false);
    mModules[inputFilePath].ast = parser.parse();
}

void Compiler::doImports(fs::path inputFilePath) {
    auto& ast = mModules[inputFilePath].ast;

    std::set<std::string> modules{ mModules[inputFilePath].moduleName };
    for (auto& node : ast) {
        if (auto import = dynamic_cast<parser::ImportStatement*>(node.get())) {
            std::string module;
            for (auto& name : import->getModule()) {
                module += name + '/';
            }

            module.pop_back();
            modules.insert(module);

            if (mMultiModules.find(module) == mMultiModules.end()) {
                mDiag.compilerError(import->getErrorToken().getStartLocation(),
                                    import->getErrorToken().getEndLocation(),
                                    std::format("could not find module '{}{}{}'", fmt::bold, module, fmt::defaults));
                std::exit(1);
            }
        }
    }

    for (auto& module : modules) {
        for (auto& file : mMultiModules[module]) {
            if (file != inputFilePath) {
                mModules[inputFilePath].scope->children.push_back(mModules[file].scope.get());
            }
        }
    }
}

void Compiler::compileModule(fs::path inputFilePath, fs::path outputFilePath) {
    auto& ast = mModules[inputFilePath].ast;
    codegen::Context ctx(std::make_unique<codegen::ModuleNode>(1, mModules[inputFilePath].moduleName));

    bool hadErrors = false;
    for (auto& node : ast) {
        node->typeCheck(mDiag, hadErrors);
    }
    if (hadErrors) {
        Log::Verbose("error occurred during type checking");
        std::exit(1);
    }

    hadErrors = false;
    for (auto& node : ast) {
        node->semanticCheck(mDiag, hadErrors, true);
    }
    if (hadErrors) {
        Log::Verbose("error occurred during semantic checking");
        std::exit(1);
    }

    addModuleBloat(ctx.getModule());

    codegen::Builder builder(ctx);
    for (auto& node : ast) {
        node->codegen(builder, ctx, mDiag, true);
    }

    codegen::PassManager passManager;
    passManager.addPass(std::make_unique<codegen::DCEPass>());

    passManager.addPass(std::make_unique<codegen::FinalizePass>());

    if (Log::verbose) passManager.addPass(std::make_unique<codegen::PrintPass>(std::cout));
    passManager.addPass(std::make_unique<codegen::CodegenPass>(outputFilePath.string()));

    passManager.runPasses(ctx.getModule());

    bool hasNatives = std::any_of(ctx.getModule()->functions.begin(), ctx.getModule()->functions.end(), [](auto& node) {
       return node->modifiers & MODULEWEB_FUNCTION_MODIFIER_NATIVE;
    });

    if (hasNatives) {
        fs::path outputHeader = outputFilePath;
        outputHeader.replace_extension(".h");

        HeaderBuilder headerBuilder(mModules[inputFilePath].moduleName);

        for (auto& func : ctx.getModule()->functions) {
            if (func->modifiers & MODULEWEB_FUNCTION_MODIFIER_NATIVE)
                headerBuilder.addFunction(func->name, func->descriptor);
        }

        std::ofstream out(outputHeader);
        headerBuilder.finalize();
        headerBuilder.emit(out);
    }
}

void Compiler::addModuleBloat(codegen::ModuleNode* module) {
    module->attributes.push_back(std::make_unique<JesusASM::Attribute<std::string_view>>("CompilerID", "Bibble-Lang 1"));
}
