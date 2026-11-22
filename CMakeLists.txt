cmake_minimum_required(VERSION 3.28)

project(SCM3 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# $<0:> stops multi-config generators (Visual Studio) from adding a per-config subdirectory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<0:>)

include(FetchContent)

set(CLI11_PRECOMPILED ON CACHE BOOL "" FORCE)
FetchContent_Declare(cli11
    GIT_REPOSITORY https://github.com/cliutils/cli11.git
    GIT_TAG v2.7.2
    SYSTEM
)
FetchContent_MakeAvailable(cli11)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.18.0.tar.gz
    SYSTEM
)
FetchContent_MakeAvailable(googletest)

add_library(SCM3_shared INTERFACE)

target_compile_options(SCM3_shared INTERFACE
    # General
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /permissive->
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wall -Wextra -Wpedantic -Wno-missing-field-initializers -Wshadow -Wnon-virtual-dtor>

    # Debug
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/Od /Ob0 /Zi /RTC1>
    $<$<AND:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>,$<CONFIG:Debug>>:-O0 -g>

    # Release
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/O2 /Oi /Gy>
    $<$<AND:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>,$<CONFIG:Release>>:-O2>
)

add_subdirectory(src/codec)
add_subdirectory(src/asm)
add_subdirectory(src/vm)

enable_testing()
add_subdirectory(tests)
