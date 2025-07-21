# Core module CMakeLists.txt
# This file recursively includes all subdirectories under the core directory


# Find required packages
find_package(yaml-cpp REQUIRED)

# Recursively collect all source files from subdirectories
file(GLOB_RECURSE CORE_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

# Create core library target
if(CORE_SOURCES)
    add_library(plib-core STATIC ${CORE_SOURCES})
    # Set include directories for shared library
    target_include_directories(plib-core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    # Link dependencies
    target_link_libraries(plib-core PUBLIC
        yaml-cpp::yaml-cpp
    )
else()
    # If no source files found, create an interface library
    add_library(plib-core INTERFACE)
    # Set include directories for interface library
    target_include_directories(plib-core INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    # Link dependencies for interface library
    target_link_libraries(plib-core PUBLIC
        yaml-cpp::yaml-cpp
    )
endif()

