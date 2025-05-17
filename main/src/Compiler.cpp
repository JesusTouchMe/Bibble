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
        mImportManager.addSearchPath(std::move(path));
    }

    void Compiler::compile() {
        if (mInput.empty()) {
            Log::Error("no input file");
            std::exit(1);
        }

        Type::Init();

        symbol::SourceFile* existing = mImportManager.findExistingSourceFile(mInput);
        if (existing == nullptr) { // The file hasn't been parsed before
            existing = mImportManager.importModule(mInput, mModuleName);

            if (existing == nullptr) {
                Log::Error(std::format("file '{}' not found", mInput.string()));
                std::exit(1);
            }
        }

        if (mOutput.empty()) {
            mOutput = mInput;
            mOutput.replace_extension(".jmod");
        }

        bool hadErrors = false;
        for (auto& node : existing->ast) {
            node->typeCheck(existing->diag, hadErrors);
        }
        if (hadErrors) {
            Log::Verbose("error occurred during type checking");
            std::exit(1);
        }

        hadErrors = false;
        for (auto& node : existing->ast) {
            node->semanticCheck(existing->diag, hadErrors, true);
        }
        if (hadErrors) {
            Log::Verbose("error occurred during semantic checking");
            std::exit(1);
        }

        auto module = std::make_unique<JesusASM::tree::ModuleNode>(1, mModuleName);

        addModuleBloat(module.get());

        codegen::Context ctx(std::move(module));
        codegen::Builder builder(ctx);

        for (auto& node : existing->ast) {
            node->codegen(builder, ctx, existing->diag, true);
        }

        if (Log::verbose) ctx.getModule()->print(std::cout);

        moduleweb::ModuleBuilder moduleBuilder;
        moduleweb::ModuleInfo moduleInfo;

        ctx.getModule()->emit(moduleBuilder, moduleInfo);

        fs::create_directories(mOutput.parent_path());

        std::string outFileName = mOutput.string();
        moduleweb::FileOutStream out(outFileName);

        moduleInfo.emit(out);
    }

    void Compiler::addModuleBloat(JesusASM::tree::ModuleNode* module) {
        module->attributes.push_back(std::make_unique<JesusASM::Attribute<std::string_view>>("CompilerID", "Bibble-Lang 1"));
    }
}