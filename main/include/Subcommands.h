// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_MAIN_INCLUDE_SUBCOMMANDS_H
#define BIBBLE_MAIN_INCLUDE_SUBCOMMANDS_H 1

#include <functional>
#include <string>
#include <vector>

namespace Bibble {
    using Subcommand = std::function<int(const std::vector<std::string>&)>;

    void InitSubcommands();

    int CallSubcommand(const std::string& subcommand, const std::vector<std::string>& args = {});
}

#endif // BIBBLE_MAIN_INCLUDE_SUBCOMMANDS_H
