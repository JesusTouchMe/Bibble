// Copyright 2025 JesusTouchMe

#include "Platform.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

#include <iostream>

namespace Bibble {
#ifdef PLATFORM_WINDOWS
    void SetupWindowsConsole() {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE) {
            std::cout << "bibble: failed to find stdout (windows specific issue)\n";
            std::cout << "bibble: console output will have weird formatting\n";
            return;
        }

        DWORD dwMode = 0;
        if (!GetConsoleMode(hOut, &dwMode)) {
            std::cout << "bibble: failed to get stdout console mode (windows specific issue)\n";
            std::cout << "bibble: console output will have weird formatting\n";
            return;
        }

        dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);

        HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
        if (hErr == INVALID_HANDLE_VALUE) {
            std::cout << "bibble: failed to find stderr (windows specific issue)\n";
            std::cout << "bibble: console output will have weird formatting\n";
            return;
        }

        if (!GetConsoleMode(hErr, &dwMode)) {
            std::cout << "bibble: failed to get stderr console mode (windows specific issue)\n";
            std::cout << "bibble: console output will have weird formatting\n";
            return;
        }

        dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hErr, dwMode);
    }
#endif
}