cmake_minimum_required(VERSION 3.20)
project(plib VERSION 1.0.0 LANGUAGES CXX)

# Load Conan generated files
if(EXISTS ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
    include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable compile_commands.json for better IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Confure dynamic library export/import macros
if(WIN32)
    set(PLIB_EXPORTS "__declspec(dllexport)")
    set(PLIB_IMPORTS "__declspec(dllimport)")
elseif(APPLE)
    set(PLIB_EXPORTS "__attribute__((visibility(\"default\")))")
    set(PLIB_IMPORTS "__attribute__((visibility(\"default\")))")
elseif(UNIX)
    set(PLIB_EXPORTS "__attribute__((visibility(\"default\")))")
    set(PLIB_IMPORTS "__attribute__((visibility(\"default\")))")
endif()

# Add compile definitions for dynamic library
add_compile_definitions(PLIB_EXPORTS)

# Set platform-specific compiler flags
if(MSVC)
    add_compile_options(/W4 /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -fPIC)
endif()

# Find dependencies
find_package(ZLIB REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(CURL REQUIRED)
find_package(asio REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(simdjson REQUIRED)
find_package(benchmark QUIET)
find_package(minizip REQUIRED)
find_package(openssl REQUIRED)
find_package(breakpad REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add subdirectories for each module
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(benchmarks)