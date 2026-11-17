cmake_minimum_required(VERSION 3.20)
project(GeoDecorCoreTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(geodecor-core-tests
    test_main.cpp
    ../src/core/LevelString.cpp
    ../src/core/ThemeAnalyzer.cpp
)
target_include_directories(geodecor-core-tests PRIVATE ../src/core)

if (MSVC)
    target_compile_options(geodecor-core-tests PRIVATE /W4 /WX)
else()
    target_compile_options(geodecor-core-tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()
add_test(NAME geodecor-core-tests COMMAND geodecor-core-tests)
