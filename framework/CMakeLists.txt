cmake_minimum_required(VERSION 3.26)

include(FetchContent)

FetchContent_Declare(
        JesusASM
        GIT_REPOSITORY https://github.com/JesusTouchMe/JesusASM.git
        GIT_TAG master
)

FetchContent_Declare(
        vlex
        GIT_REPOSITORY https://github.com/JesusTouchMe/vlex.git
        GIT_TAG master
)

FetchContent_MakeAvailable(JesusASM)
FetchContent_MakeAvailable(vlex)

set(SOURCES
        src/diagnostic/Diagnostic.cpp
        src/type/Type.cpp
        src/type/BooleanType.cpp
        src/type/FunctionType.cpp
        src/type/IntegerType.cpp
        src/type/CharType.cpp
        src/type/HandleType.cpp
        src/type/VoidType.cpp
        src/type/ClassType.cpp
        src/symbol/Scope.cpp
        src/symbol/Import.cpp
        src/parser/ImportParser.cpp
        src/parser/ast/Node.cpp
        src/parser/ast/global/Function.cpp
        src/parser/ast/global/ClassDeclaration.cpp
        src/parser/Parser.cpp
        src/parser/ast/expression/IntegerLiteral.cpp
        src/parser/ast/expression/BinaryExpression.cpp
        src/parser/ast/expression/CallExpression.cpp
        src/parser/ast/expression/VariableExpression.cpp
        src/codegen/Context.cpp
        src/codegen/Builder.cpp)
set(HEADERS
        include/Bible/diagnostic/Diagnostic.h
        include/Bible/type/Type.h
        include/Bible/type/BooleanType.h
        include/Bible/type/FunctionType.h
        include/Bible/type/IntegerType.h
        include/Bible/type/CharType.h
        include/Bible/type/HandleType.h
        include/Bible/type/VoidType.h
        include/Bible/type/ClassType.h
        include/Bible/parser/Parser.h
        include/Bible/symbol/Scope.h
        include/Bible/parser/ImportParser.h
        include/Bible/symbol/Import.h
        include/Bible/parser/ast/Node.h
        include/Bible/codegen/Builder.h
        include/Bible/codegen/Context.h
        include/Bible/parser/ast/global/Function.h
        include/Bible/parser/ast/global/ClassDeclaration.h
        include/Bible/parser/ast/expression/Integerliteral.h
        include/Bible/parser/ast/expression/BinaryExpression.h
        include/Bible/parser/ast/expression/CallExpression.h
        include/Bible/parser/ast/expression/VariableExpression.h
        include/Bible/codegen/Type.h)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_library(bible-framework ${SOURCES} ${HEADERS})
add_library(Bible::framework ALIAS bible-framework)

vlex_generate(bible-framework src/lexer/Lexer.vlex src/lexer include/Bible/lexer)

target_include_directories(bible-framework
    PUBLIC
        include
)

target_compile_features(bible-framework PUBLIC cxx_std_20)

if(WIN32)
    target_compile_definitions(bible-framework PUBLIC PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(bible-framework PUBLIC PLATFORM_MACOS)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(bible-framework PUBLIC PLATFORM_LINUX)
endif()

add_dependencies(bible-framework JesusASM::framework)
target_link_libraries(bible-framework PUBLIC JesusASM::framework)