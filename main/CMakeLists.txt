cmake_minimum_required(VERSION 3.26)

include(FetchContent)

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0  # Check for the latest version
)

FetchContent_MakeAvailable(tomlplusplus)

set(SOURCES
    src/main.cpp
    src/Platform.cpp
    src/Subcommands.cpp
        src/Compiler.cpp)

set(HEADERS
    include/Platform.h
    include/Subcommands.h
        include/Compiler.h)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_executable(Bibble ${SOURCES} ${HEADERS})
add_executable(Bibble::main ALIAS Bibble)

target_include_directories(Bibble
    PUBLIC
        include
)

target_include_directories(Bibble PRIVATE ${tomlplusplus_SOURCE_DIR})

target_compile_features(Bibble PUBLIC cxx_std_23)
target_link_libraries(Bibble PRIVATE Bibble::framework)
target_link_libraries(Bibble PRIVATE JesusVM::framework)