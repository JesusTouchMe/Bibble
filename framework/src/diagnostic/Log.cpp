// Copyright 2025 JesusTouchMe

#include "Bibble/diagnostic/Diagnostic.h"
#include "Bibble/diagnostic/Log.h"

namespace Log {
    bool verbose = false;

    void Info(std::string_view message) {
        std::cout << fmt::bold << "info: " << fmt::defaults << message << "\n";
    }

    void Verbose(std::string_view message) {
        if (verbose) {
            std::cout << fmt::bold << fmt::cyan << "verbose: " << fmt::defaults << message << "\n";
        }
    }

    void Warning(std::string_view message) {
        std::cout << fmt::bold << fmt::yellow << "warning: " << fmt::defaults << message << "\n";
    }

    void Error(std::string_view message) {
        std::cout << fmt::bold << fmt::red << "error: " << fmt::defaults << message << "\n";
    }
}