// Copyright 2025 JesusTouchMe

#include "Subcommands.h"

#include "Bibble/diagnostic/Log.h"

#include <toml.hpp>

#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

namespace commands {
    constexpr std::string_view mainConfigFile = "bibbler.toml";

    int help(const std::vector<std::string>& args) {
        std::cout << "help";
        return 0;
    }

    template <typename T>
    constexpr T GetInput(std::string_view prompt) {
        T input;
        std::cout << prompt;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::getline(std::cin, input);
        return input;
    }

    int init(const std::vector<std::string>& args) {
        if (fs::exists(mainConfigFile)) {
            Log::Error("a project has already been configured in this directory");
            return 1;
        }

        toml::table config;

        auto projectName = GetInput<std::string>("Project name: ");
        config.insert("ProjectName", std::move(projectName));

        auto projectVersion = GetInput<std::string>("Project version (1.0.0): ");
        if (projectVersion.empty()) projectVersion = "1.0.0";
        config.insert("ProjectVersion", std::move(projectVersion));

        auto projectDescription = GetInput<std::string>("Project description: ");
        config.insert("ProjectDescription", std::move(projectDescription));

        toml::table build;

        build.insert("SourceDir", "src");
        build.insert("BuildDir", "build");

        config.insert("build", std::move(build));

        toml::table runtime;

        runtime.insert("VMExecutable", "jesusvm");
        runtime.insert("StandardVersion", 1);
        runtime.insert("ModuleSearchPaths", toml::array());
        runtime.insert("PluginSearchPaths", toml::array());

        config.insert("runtime", std::move(runtime));

        std::ofstream out((std::string(mainConfigFile)));
        out << config;

        Log::Info("initialized project");
        return 0;
    }
}

namespace Bibble {
    std::unordered_map<std::string, Subcommand> subcommands;

    void InitSubcommands() {
        subcommands["help"] = commands::help;
        subcommands["init"] = commands::init;
    }

    int CallSubcommand(const std::string& subcommand, const std::vector<std::string>& args) {
        auto it = subcommands.find(subcommand);

        if (it != subcommands.end()) {
            return it->second(args);
        }

        Log::Error("Unknown subcommand");
        return -1;
    }
}