cmake_minimum_required(VERSION 3.26)

include(FetchContent)

FetchContent_Declare(
        JesusASM
        GIT_REPOSITORY https://github.com/JesusTouchMe/JesusASM.git
        GIT_TAG master
)

FetchContent_Declare(
        vlex
        GIT_REPOSITORY https://github.com/viper-org/vlex.git
        GIT_TAG master
)

FetchContent_MakeAvailable(JesusASM)
FetchContent_MakeAvailable(vlex)

set(SOURCES)
set(HEADERS)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_library(bible-framework ${SOURCES} ${HEADERS})
add_library(Bible::framework ALIAS bible-framework)

vlex_generate(bible-framework src/lexer/Lexer.vlex src/lexer include/Bible/lexer)

target_include_directories(bible-framework
    PUBLIC
        include
)

target_compile_features(bible-framework PUBLIC cxx_std_20)

add_dependencies(bible-framework JesusASM::framework)
target_link_libraries(bible-framework PUBLIC JesusASM::framework)