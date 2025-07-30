// Copyright 2025 JesusTouchMe

#include "Compiler.h"
#include "Platform.h"

#include "Bibble/diagnostic/Log.h"

#include <vector>

using namespace Bibble;

void PrintHelp() {

}

int main(int argc, char** argv) {
#ifdef PLATFORM_WINDOWS
    SetupWindowsConsole();
#endif

    if (argc < 2) {
        PrintHelp();
        return 0;
    }

    std::string subcommand = argv[1];
    std::vector<std::string> args(argv + 2, argv + argc);

    for (const auto& arg : args) {
        if (arg == "-V" || arg == "--verbose") {
            Log::verbose = true;
            break;
        }
    }

    diagnostic::Diagnostics diag;

    if (subcommand == "help") {
        PrintHelp();
    } else if (subcommand == "version") {
        std::cout << "bibble v0.1\n";
    } else if (subcommand == "build") {
        Compiler compiler(diag);
        compiler.build();
    }

    return 0;
}