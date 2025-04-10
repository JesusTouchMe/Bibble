// Copyright 2025 JesusTouchMe

#include "Platform.h"
#include "Subcommands.h"

#include "Bibble/diagnostic/Log.h"

using namespace Bibble;

int main(int argc, char** argv) {
#ifdef PLATFORM_WINDOWS
    SetupWindowsConsole();
#endif

    InitSubcommands();

    if (argc < 2) {
        return CallSubcommand("help");
    }

    std::string subcommand = argv[1];
    std::vector<std::string> args(argv + 2, argv + argc);

    for (const auto& arg : args) {
        if (arg == "-V" || arg == "--verbose") {
            Log::verbose = true;
            break;
        }
    }

    return CallSubcommand(subcommand, args);
}