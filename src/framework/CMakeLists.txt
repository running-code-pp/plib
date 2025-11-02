
# Recursively collect all source files from src/ and all headers from include/
file(GLOB_RECURSE FRAMEWORK_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)
file(GLOB_RECURSE FRAMEWORK_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

# Create framework library target
if(FRAMEWORK_SOURCES)
    add_library(plib-framework STATIC ${FRAMEWORK_SOURCES} ${FRAMEWORK_HEADERS})
    # Set include directories for shared library
    target_include_directories(plib-framework PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    # Link dependencies
    target_link_libraries(plib-core PUBLIC
        plib-core
    )
else()
    # If no source files found, create an interface library
    add_library(plib-framework INTERFACE)
    # Set include directories for interface library
    target_include_directories(plib-framework INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    # Link dependencies for interface library
    target_link_libraries(plib-framework INTERFACE
    plib-core
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