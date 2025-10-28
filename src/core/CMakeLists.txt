# Core module CMakeLists.txt

# Find required packages
find_package(yaml-cpp REQUIRED)

# Recursively collect all source files from src/ and all headers from include/
file(GLOB_RECURSE CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)
file(GLOB_RECURSE CORE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

# Create core library target
if(CORE_SOURCES)
    add_library(plib-core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
    # Set include directories for shared library
    target_include_directories(plib-core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    # Link dependencies
    target_link_libraries(plib-core PUBLIC
        yaml-cpp::yaml-cpp
        spdlog::spdlog
        simdjson::simdjson
        nlohmann_json::nlohmann_json
    )
else()
    # If no source files found, create an interface library
    add_library(plib-core INTERFACE)
    # Set include directories for interface library
    target_include_directories(plib-core INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    # Link dependencies for interface library
    target_link_libraries(plib-core INTERFACE
        yaml-cpp::yaml-cpp
        simdjson::simdjson
    )
endif()

# Visual Studio: group headers and sources by their real directory structure
foreach(header ${CORE_HEADERS})
    file(RELATIVE_PATH rel_path "${CMAKE_CURRENT_SOURCE_DIR}/include" "${header}")
    get_filename_component(dir "${rel_path}" PATH)
    if(dir)
        source_group("Header Files\\${dir}" FILES "${header}")
    else()
        source_group("Header Files" FILES "${header}")
    endif()
endforeach()

foreach(source ${CORE_SOURCES})
    file(RELATIVE_PATH rel_path "${CMAKE_CURRENT_SOURCE_DIR}/src" "${source}")
    get_filename_component(dir "${rel_path}" PATH)
    if(dir)
        source_group("Source Files\\${dir}" FILES "${source}")
    else()
        source_group("Source Files" FILES "${source}")
    endif()
endforeach()