// Copyright 2025 JesusTouchMe

#include "Compiler.h"

#include "Bibble/diagnostic/Diagnostic.h"
#include "Bibble/diagnostic/Log.h"

#include "Bibble/lexer/Lexer.h"

#include "Bibble/parser/Parser.h"

#include "Bibble/symbol/Import.h"
#include "Bibble/symbol/Scope.h"

#include "Bibble/type/Type.h"

#include <format>
#include <fstream>
#include <sstream>

namespace Bibble {
    void Compiler::setModuleName(std::string moduleName) {
        mModuleName = std::move(moduleName);
    }

    void Compiler::setInput(fs::path input) {
        mInput = std::move(input);
    }

    void Compiler::setOutput(fs::path output) {
        mOutput = std::move(output);
    }

    void Compiler::addImportPath(fs::path path) {
        mImportPaths.push_back(std::move(path));
    }


    void Compiler::compile() {
        if (mInput.empty()) {
            Log::Error("no input file");
            std::exit(1);
        }

        if (mOutput.empty()) {
            mOutput = mInput;
            mOutput.replace_extension(".jmod");
        }

        std::ifstream inputStream(mInput);

        if (!inputStream.is_open()) {
            Log::Error(std::format("could not find file '{}'", mInput.string()));
            std::exit(1);
        }

        fs::path fullInputFilePath = fs::current_path() / mInput;
        std::string fullInputPathName = fullInputFilePath.string();

        std::stringstream buffer;
        buffer << inputStream.rdbuf();
        std::string text = buffer.str();

        diagnostic::Diagnostics diag;
        diag.setText(text);

        Type::Init();

        lexer::Lexer lexer(text, fullInputPathName);
        auto tokens = lexer.lex();

        symbol::ImportManager importManager;
        for (const auto& path : mImportPaths) {
            importManager.addModulePath(path);
        }

        symbol::ScopePtr globalScope = std::make_unique<symbol::Scope>(nullptr, "", true);
        parser::Parser parser(tokens, diag, importManager, globalScope.get());

        auto ast = parser.parse();

        bool hadErrors = false;
        for (auto& node : ast) {
            node->typeCheck(diag, hadErrors);
        }
        if (hadErrors) std::exit(1);

        hadErrors = false;
        for (auto& node : ast) {
            node->semanticCheck(diag, hadErrors, true);
        }
        if (hadErrors) std::exit(1);

        auto module = std::make_unique<JesusASM::tree::ModuleNode>(1, mModuleName);

        addModuleBloat(module.get());

        codegen::Context ctx(std::move(module));
        codegen::Builder builder(ctx);

        for (auto& node : ast) {
            node->codegen(builder, ctx, diag);
        }

        std::vector<JesusASM::tree::AbstractInsnNode*> debug;
        for (auto it = builder.mInsertPoint->getFirst(); it != nullptr; it = it->getNext()) {
            debug.push_back(it);
        }

        moduleweb::ModuleBuilder moduleBuilder;
        moduleweb::ModuleInfo moduleInfo;

        ctx.getModule()->emit(moduleBuilder, moduleInfo);

        if (Log::verbose) moduleInfo.print();

        fs::create_directories(mOutput.parent_path());

        std::string outFileName = mOutput.string();
        moduleweb::FileOutStream out(outFileName);

        moduleInfo.emit(out);

        Type::Reset();
    }

    void Compiler::addModuleBloat(JesusASM::tree::ModuleNode* module) {
        module->attributes.push_back(std::make_unique<JesusASM::Attribute<std::string_view>>("CompilerID", "Bibble-Lang 1"));
    }
}