// Copyright 2025 JesusTouchMe

// Borrowed from viper-lang: https://github.com/viper-org/viper-lang/blob/master/framework/include/diagnostic/Diagnostic.h

#include "Bible/diagnostic/Diagnostic.h"

#include "Bible/lexer/Token.h"

#include <format>
#include <iostream>

namespace diagnostic {
    void Diagnostics::setImported(bool imported) {
        mImported = imported;
    }

    void Diagnostics::setFileName(std::string fileName) {
        mFileName = std::move(fileName);
    }

    void Diagnostics::setErrorSender(std::string sender) {
        mSender = std::move(sender);
    }

    void Diagnostics::setText(std::string text) {
        mText = std::move(text);
    }

    void Diagnostics::fatalError(std::string_view message) {
        std::cerr << std::format("{}{}: {}fatal error: {}{}\n", fmt::bold, mSender, fmt::red, fmt::defaults, message);

        std::exit(EXIT_FAILURE);
    }

    void Diagnostics::compilerError(lexer::SourceLocation start, lexer::SourceLocation end, std::string_view message) {
        unsigned int lineStart = getLinePosition(start.line - 1);
        unsigned int lineEnd = getLinePosition(end.line) - 1;

        end.position += 1;
        std::string before = mText.substr(lineStart, start.position - lineStart);
        std::string error = mText.substr(start.position, end.position - start.position);
        std::string after = mText.substr(end.position, lineEnd - end.position);
        std::string spacesBefore = std::string(std::to_string(start.line).length(), ' ');
        std::string spacesAfter = std::string(before.length(), ' ');

        std::string imported = mImported ? " in imported file" : "";

        std::cerr << std::format("{}{}:{}:{} {}error{}: {}{}\n", fmt::bold, mFileName, start.line, start.col, fmt::red, imported, fmt::defaults, message);
        std::cerr << std::format("    {} | {}{}{}{}{}{}\n", start.line, before, fmt::bold, fmt::red, error, fmt::defaults, after);
        std::cerr << std::format("    {} | {}{}{}^{}{}\n", spacesBefore, spacesAfter, fmt::bold, fmt::red, std::string(error.length()-1, '~'), fmt::defaults);

        std::exit(EXIT_FAILURE);
    }

    void Diagnostics::compilerWarning(lexer::SourceLocation start, lexer::SourceLocation end, std::string_view message) {
        unsigned int lineStart = getLinePosition(start.line - 1);
        unsigned int lineEnd = getLinePosition(end.line) - 1;

        end.position += 1;
        std::string before = mText.substr(lineStart, start.position - lineStart);
        std::string error = mText.substr(start.position, end.position - start.position);
        std::string after = mText.substr(end.position, lineEnd - end.position);
        std::string spacesBefore = std::string(std::to_string(start.line).length(), ' ');
        std::string spacesAfter = std::string(before.length(), ' ');

        std::string imported = mImported ? " in imported file" : "";

        std::cerr << std::format("{}{}:{}:{} {}warning{}: {}{}\n", fmt::bold, mFileName, start.line, start.col, fmt::yellow, imported, fmt::defaults, message);
        std::cerr << std::format("    {} | {}{}{}{}{}{}\n", start.line, before, fmt::bold, fmt::yellow, error, fmt::defaults, after);
        std::cerr << std::format("    {} | {}{}{}^{}{}\n", spacesBefore, spacesAfter, fmt::bold, fmt::yellow, std::string(error.length()-1, '~'), fmt::defaults);
    }


    unsigned int Diagnostics::getLinePosition(unsigned int lineNumber) {
        unsigned int line = 0;
        for (int i = 0; i < lineNumber; i++) {
            while(mText[line] != '\n') {
                line++;
            }
            line++;
        }

        return line;
    }
}