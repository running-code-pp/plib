# Examples directory CMakeLists.txt

# Find all source files recursively
file(GLOB_RECURSE EXAMPLE_SOURCES "*.cpp" "*.c")

# Only create executable if source files exist
if(EXAMPLE_SOURCES)
    # Example executable
    add_executable(plib_example ${EXAMPLE_SOURCES})
    
    # Link with plib
    target_link_libraries(plib_example
        plib
    )
    
    # Include directories
    target_include_directories(plib_example PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    )
    
    # Set compile definitions for importing from DLL
    if(WIN32)
        target_compile_definitions(plib_example PUBLIC PLIB_IMPORTS)
    endif()
else()
    message(WARNING "No source files found in examples directory")
endif() 