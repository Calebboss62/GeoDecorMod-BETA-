
cmake_minimum_required(VERSION 3.21)

project(GeoDecorStudio VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK!")
endif()

message(STATUS "Found Geode SDK: $ENV{GEODE_SDK}")
add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/core/LevelString.cpp
    src/core/ThemeAnalyzer.cpp
    src/core/BlockDNA.cpp
    src/integration/GmdImport.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
setup_geode_mod(${PROJECT_NAME})
