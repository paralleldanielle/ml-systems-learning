cmake_minimum_required(VERSION 3.24)
project(MlSystemsLearning LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(LEARN_USE_REFERENCE "Build exercises against the teacher reference solution instead of the student code" OFF)
option(LEARN_CLANG_TIDY "Run clang-tidy on exercise code while compiling" OFF)
set(LEARN_SANITIZERS "" CACHE STRING "Comma-separated -fsanitize= list, e.g. address,undefined or thread")

if(LEARN_SANITIZERS)
    add_compile_options(-fsanitize=${LEARN_SANITIZERS} -fno-omit-frame-pointer -fno-sanitize-recover=all)
    add_link_options(-fsanitize=${LEARN_SANITIZERS})
endif()

include(cmake/LearnTargets.cmake)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

enable_testing()

add_subdirectory(exercises/01-sensor-sync)
