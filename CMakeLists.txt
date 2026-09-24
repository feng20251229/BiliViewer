cmake_minimum_required(VERSION 3.20)
project(BiliNative LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSYS2 MinGW 的路径（按你实际改）
set(MSYS2_ROOT "E:/soft intallation/MSYS2/mingw64")

# ============ C++ 动态库（给 C# 用）============
add_library(BiliNative SHARED
    src/connect.cpp
    src/data.cpp
    src/bili_export.cpp
)

target_include_directories(BiliNative PRIVATE
    ${MSYS2_ROOT}/include
    src
)

target_link_directories(BiliNative PRIVATE
    ${MSYS2_ROOT}/lib
)

target_link_libraries(BiliNative PRIVATE
    curl
    cjson
)

target_compile_definitions(BiliNative PRIVATE
    BILI_EXPORTS
)

# 输出到项目根的 cpp-build/
set_target_properties(BiliNative PROPERTIES
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/cpp-build"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/cpp-build"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/cpp-build"
)

# ============ 命令行版 main.exe（可选）============
add_executable(bili_main
    src/connect.cpp
    src/data.cpp
    src/main.cpp
)

target_include_directories(bili_main PRIVATE
    ${MSYS2_ROOT}/include
    src
)

target_link_directories(bili_main PRIVATE
    ${MSYS2_ROOT}/lib
)

target_link_libraries(bili_main PRIVATE
    curl
    cjson
)

set_target_properties(bili_main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/cpp-build"
)