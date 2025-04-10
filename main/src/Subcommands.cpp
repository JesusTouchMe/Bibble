// Copyright 2025 JesusTouchMe

#include "Compiler.h"
#include "Subcommands.h"

#include "Bibble/diagnostic/Log.h"

#include <toml.hpp>

#include <JesusVM/JesusVM.h>
#include <JesusVM/Preload.h>

#include <JesusVM/constpool/ConstantFunc.h>

#include <JesusVM/executors/Threading.h>

#include <filesystem>
#include <format>
#include <sstream>
#include <unordered_map>

namespace fs = std::filesystem;

namespace commands {
    constexpr std::string_view mainConfigFile = "bibbler.toml";
    constexpr std::string_view defaultSrcDir = "src";
    constexpr std::string_view defaultBuildDir = ".bibble/build";
    constexpr std::string_view defaultTestDir = "test";

    int help(const std::vector<std::string>& args) {
        std::cout << "help";
        return 0;
    }

    template <typename T>
    constexpr T GetInput(std::string_view prompt) {
        T input;
        std::cout << prompt << std::flush;

        std::string line;
        std::getline(std::cin, line);

        if constexpr (std::is_same_v<T, std::string>) {
            return line;
        } else {
            std::istringstream iss(line);
            if (!(iss >> input)) {
                Log::Error("input issue");
                std::exit(1);
            }
        }

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

        build.insert("SourceDir", defaultSrcDir);
        build.insert("BuildDir", defaultBuildDir);

        config.insert("build", std::move(build));

        toml::table runtime;

        auto mainModule = GetInput<std::string>("Main module: ");
        runtime.insert("MainModule", std::move(mainModule));

        runtime.insert("VMExecutable", "bundled");
        runtime.insert("StandardVersion", 1);
        runtime.insert("ModuleSearchPaths", toml::array());
        runtime.insert("PluginSearchPaths", toml::array());

        config.insert("runtime", std::move(runtime));

        auto useTestsText = GetInput<std::string>("Use tests? (y/n) ");
        std::transform(useTestsText.begin(), useTestsText.end(), useTestsText.begin(), [](auto c) { return std::tolower(c); });

        while (useTestsText != "y" && useTestsText != "n") {
            useTestsText = GetInput<std::string>("(y/n) ");
            std::transform(useTestsText.begin(), useTestsText.end(), useTestsText.begin(), [](auto c) { return std::tolower(c); });
        }

        bool useTests = useTestsText == "y";
        if (useTests) {
            toml::table test;

            test.insert("TestDir", defaultTestDir);

            config.insert("test", std::move(test));

            fs::create_directories(defaultTestDir);
        }

        std::ofstream out((std::string(mainConfigFile)));
        out << config;

        fs::create_directories(defaultSrcDir);
        fs::create_directories(defaultBuildDir);

        Log::Info("initialized project");
        return 0;
    }

    static void Compile(Bibble::Compiler& compiler, const fs::path& srcDir, const fs::path& input, const fs::path& output) {
        if (fs::is_regular_file(input) && input.extension() == ".bibble") {
            if (fs::exists(output)) {
                auto inputTime = fs::last_write_time(input);
                auto outputTime = fs::last_write_time(output);

                if (inputTime <= outputTime) return; // already compiled. skipping
            }

            std::string moduleName = fs::relative(input, srcDir).string();
            std::replace(moduleName.begin(), moduleName.end(), '\\', '.');
            std::replace(moduleName.begin(), moduleName.end(), '/', '.');

            moduleName = moduleName.substr(0, moduleName.find_last_of('.'));

            compiler.setModuleName(std::move(moduleName));
            compiler.setInput(input);
            compiler.setOutput(output);

            compiler.addImportPath(srcDir);

            compiler.compile();
        }
    }

    static void RunCompiler(Bibble::Compiler& compiler, const fs::path& srcDir, const fs::path& buildDir) {
        for (const auto& entry : fs::recursive_directory_iterator(srcDir)) {
            fs::path output = buildDir / srcDir / fs::relative(entry, srcDir);
            output.replace_extension(".jmod");

            Compile(compiler, srcDir, entry, output);
        }
    }

    int build(const std::vector<std::string>& args) {
        if (!fs::exists(mainConfigFile)) {
            Log::Error("no bibbler.toml was found in the current directory");
            return 1;
        }

        try {
            toml::table config = toml::parse_file(mainConfigFile);

            auto build = config["build"].as_table();
            if (build) {
                fs::path buildDir = (*build)["BuildDir"].value_or(defaultBuildDir);
                fs::path srcDir = (*build)["SourceDir"].value_or(defaultSrcDir);

                Bibble::Compiler compiler;
                RunCompiler(compiler, srcDir, buildDir);
            } else {
                Log::Error("no build config found");
                return 1;
            }

            return 0;
        } catch (const toml::parse_error& error) {
                Log::Error(std::format("error parsing project configuration: {}", error.description()));
            return 1;
        }
    }

    int run(const std::vector<std::string>& args) {
        if (!fs::exists(mainConfigFile)) {
            Log::Error("no bibbler.toml was found in the current directory");
            return 1;
        }

        try {
            toml::table config = toml::parse_file(mainConfigFile);

            auto build = config["build"].as_table();
            if (!build) {
                Log::Error("no build config found");
                return 1;
            }

            fs::path buildDir = (*build)["BuildDir"].value_or(defaultBuildDir);
            fs::path srcDir = (*build)["SourceDir"].value_or(defaultSrcDir);

            Bibble::Compiler compiler;
            RunCompiler(compiler, srcDir, buildDir); // build the code before running

            auto runtime = config["runtime"].as_table();
            if (!runtime) {
                Log::Error("no runtime configured");
                return 1;
            }

            std::string mainModule = (*runtime)["MainModule"].value_or("");
            if (mainModule.empty()) {
                Log::Error("run needs MainModule to have a value");
                return 1;
            }

            std::string vmExecutable = (*runtime)["VMExecutable"].value_or("bundled");
            auto moduleSearchPaths = (*runtime)["ModuleSearchPaths"].as_array();
            auto pluginSearchPaths = (*runtime)["PluginSearchPaths"].as_array();

            if (vmExecutable == "bundled") {
                JesusVM::Init();

                JesusVM::Linker::Init();
                JesusVM::Linker::AddPath((buildDir / srcDir).string());
                JesusVM::Linker::AddPluginPath((buildDir / srcDir).string()); //TODO: resource dir or whatever

                if (moduleSearchPaths) {
                    for (const auto& path : *moduleSearchPaths) {
                        JesusVM::Linker::AddPath(path.value_or(""));
                    }
                }
                if (pluginSearchPaths) {
                    for (const auto& path : *pluginSearchPaths) {
                        JesusVM::Linker::AddPluginPath(path.value_or(""));
                    }
                }

                JesusVM::Threading::Init();

                JesusVM::Preload::PreloadSystemModules();

                JesusVM::Module* mainModuleVM = JesusVM::Linker::LoadModule(nullptr, mainModule);
                if (mainModuleVM == nullptr) {
                    Log::Error("main module does not exist");
                    return 1;
                }

                JesusVM::Function* mainFunction = mainModuleVM->getFunction("main", "()V");
                if (mainFunction == nullptr) {
                    Log::Error("main module has no main function (void main())");
                    return 1;
                }

                mainFunction->invoke<void>();

                JesusVM::Threading::WaitForAllThreads();

                return 0;
            } else {
                Log::Error("TODO: implement different vm exe");
                return -1;
            }
        } catch (const toml::parse_error& error) {
            Log::Error(std::format("error parsing project configuration: {}", error.description()));
            return 1;
        }
    }

    int test(const std::vector<std::string>& args) {

    }
}

namespace Bibble {
    std::unordered_map<std::string, Subcommand> subcommands;

    void InitSubcommands() {
        subcommands["help"] = commands::help;
        subcommands["init"] = commands::init;
        subcommands["build"] = commands::build;
        subcommands["run"] = commands::run;
        subcommands["test"] = commands::test;
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