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
        src/codegen/Builder.cpp
    src/diagnostic/Log.cpp
        src/parser/ast/expression/UnaryExpression.cpp)
set(HEADERS
    include/Bibble/diagnostic/Diagnostic.h
    include/Bibble/type/Type.h
    include/Bibble/type/BooleanType.h
    include/Bibble/type/FunctionType.h
    include/Bibble/type/IntegerType.h
    include/Bibble/type/CharType.h
    include/Bibble/type/HandleType.h
    include/Bibble/type/VoidType.h
    include/Bibble/type/ClassType.h
    include/Bibble/parser/Parser.h
    include/Bibble/symbol/Scope.h
    include/Bibble/parser/ImportParser.h
    include/Bibble/symbol/Import.h
    include/Bibble/parser/ast/Node.h
    include/Bibble/codegen/Builder.h
    include/Bibble/codegen/Context.h
    include/Bibble/parser/ast/global/Function.h
    include/Bibble/parser/ast/global/ClassDeclaration.h
    include/Bibble/parser/ast/expression/Integerliteral.h
    include/Bibble/parser/ast/expression/BinaryExpression.h
    include/Bibble/parser/ast/expression/CallExpression.h
    include/Bibble/parser/ast/expression/VariableExpression.h
    include/Bibble/codegen/Type.h
    include/Bibble/diagnostic/Log.h
        include/Bibble/parser/ast/expression/UnaryExpression.h)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_library(bible-framework ${SOURCES} ${HEADERS})
add_library(Bibble::framework ALIAS bible-framework)

vlex_generate(bible-framework src/lexer/Lexer.vlex src/lexer include/Bibble/lexer)

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