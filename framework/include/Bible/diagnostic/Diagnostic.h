// Copyright 2025 JesusTouchMe

// Borrowed from viper-lang: https://github.com/viper-org/viper-lang/blob/master/framework/include/diagnostic/Diagnostic.h

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_DIAGNOSTIC_DIAGNOSTIC_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_DIAGNOSTIC_DIAGNOSTIC_H

#include <string>

namespace lexer {
    class SourceLocation;
}

namespace fmt {
    constexpr std::string_view bold     = "\x1b[1m";
    constexpr std::string_view red      = "\x1b[31m";
    constexpr std::string_view yellow   = "\x1b[93m";
    constexpr std::string_view defaults = "\x1b[0m";
}

namespace diagnostic {
    class Diagnostics {
    public:
        void setImported(bool imported);
        void setFileName(std::string fileName);
        void setErrorSender(std::string sender);
        void setText(std::string text);

        [[noreturn]] void fatalError(std::string_view message);

        [[noreturn]] void compilerError(lexer::SourceLocation start, lexer::SourceLocation end, std::string_view message);
        void compilerWarning(lexer::SourceLocation start, lexer::SourceLocation end, std::string_view message);

    private:
        std::string mFileName;
        std::string mSender;
        std::string mText;
        bool mImported{ false };

        unsigned int getLinePosition(unsigned int lineNumber);
    };
}

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_DIAGNOSTIC_DIAGNOSTIC_H
