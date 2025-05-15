// Copyright 2025 JesusTouchMe

#include "Bibble/diagnostic/Assert.h"
#include "Bibble/diagnostic/Log.h"

#include <format>

namespace diagnostic {
    void AssertFail(const char* message, const char* file, unsigned int line) {
        Log::Error(std::format("Assertion failed: {}, file {}, line {}", message, file, line));
        std::exit(3);
    }
}