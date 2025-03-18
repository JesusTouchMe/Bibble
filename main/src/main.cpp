// Copyright 2025 JesusTouchMe

#include "Bible/diagnostic/Diagnostic.h"

#include "Bible/lexer/Lexer.h"
#include "Bible/lexer/Token.h"

#include "Bible/type/Type.h"

#include <filesystem>
#include <format>
#include <fstream>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    diagnostic::Diagnostics diag;
    diag.setErrorSender("bible");

    if (argc < 2) {
        diag.fatalError("no input files");
    }

    Type::Init();

    for (int i = 1; i < argc; i++) {
        std::string_view arg = argv[i];
        fs::path file = arg;

        if (!fs::exists(file)) {
            diag.fatalError(std::format("{}: no such file or directory", arg));
        }
        diag.setFileName(std::string(arg));

        fs::path outFile = file;
        outFile.replace_extension("jmod");

        std::ifstream input(file);
        std::stringstream buffer;
        buffer << input.rdbuf();

        std::string text = buffer.str();

        lexer::Lexer lexer(text, arg);

        std::vector<lexer::Token> tokens = lexer.lex();


    }

    return 0;
}