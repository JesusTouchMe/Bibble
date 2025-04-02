// Copyright 2025 JesusTouchMe

// Borrowed from viper-lang: https://github.com/viper-org/viper-lang/blob/master/framework/include/diagnostic/Diagnostic.h

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_DIAGNOSTIC_DIAGNOSTIC_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_DIAGNOSTIC_DIAGNOSTIC_H

#include <string>
#include <vector>

namespace lexer {
    class SourceLocation;
}

namespace fmt {
    constexpr std::string_view bold     = "\x1b[1m";
    constexpr std::string_view red      = "\x1b[31m";
    constexpr std::string_view yellow   = "\x1b[93m";
    constexpr std::string_view cyan     = "\x1b[36m";
    constexpr std::string_view defaults = "\x1b[0m";
}

namespace diagnostic {
    class Diagnostics {
    public:
        Diagnostics();

        void setImported(bool imported);
        void setWarning(bool enable, std::string_view warning);
        void setText(std::string_view text);

        [[noreturn]] void fatalError(std::string_view message);

        void compilerError(lexer::SourceLocation start, lexer::SourceLocation end, std::string_view message);
        void compilerWarning(std::string_view type, lexer::SourceLocation start, lexer::SourceLocation end, std::string_view message);

    private:
        std::string_view mText;
        std::vector<std::string_view> mWarnings;
        bool mImported{ false };

        unsigned int getLinePosition(unsigned int lineNumber);
    };
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_DIAGNOSTIC_DIAGNOSTIC_H
