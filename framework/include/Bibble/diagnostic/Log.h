// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_DIAGNOSTIC_LOG_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_DIAGNOSTIC_LOG_H 1

#include "Bibble/lexer/SourceLocation.h"

#include <iostream>

// Basic logging. Not related to the compiler

namespace Log {
    extern bool verbose;

    void Info(std::string_view message);

    void Verbose(std::string_view message);

    void Warning(std::string_view message);

    void Error(std::string_view message);
}

#endif // BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_DIAGNOSTIC_LOG_H
