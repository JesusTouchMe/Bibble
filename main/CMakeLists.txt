cmake_minimum_required(VERSION 3.26)

set(SOURCES "src/main.cpp")

set(HEADERS)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_executable(Bible ${SOURCES} ${HEADERS})
add_executable(Bible::main ALIAS Bible)

target_include_directories(Bible
    PUBLIC
        include
)

target_compile_features(Bible PUBLIC cxx_std_20)
target_link_libraries(Bible Bible::framework)