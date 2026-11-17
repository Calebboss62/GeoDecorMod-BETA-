cmake_minimum_required(VERSION 3.21)
project(GeoDecorStudio VERSION 0.2.0 LANGUAGES CXX)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/core/LevelString.cpp
    src/core/ThemeAnalyzer.cpp
    src/integration/GmdImport.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
setup_geode_mod(${PROJECT_NAME})
