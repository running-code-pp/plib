# Source directory CMakeLists.txt

# Compilation options for all modules
option(FRAMEWORK_ENABLED "Enable FRAMEWORK module" ON)
option(NETWORK_ENABLED "Enable NETWORK module" ON)

# Add subdirectories for each module
add_subdirectory(core)

if(FRAMEWORK_ENABLED)
    add_subdirectory(framework)
endif()

if(NETWORK_ENABLED)
    add_subdirectory(network)
endif()
