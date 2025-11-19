# Source directory CMakeLists.txt

# Compilation options for all modules
option(NETWORK_ENABLED "Enable NETWORK module" ON)

# Add subdirectories for each module
add_subdirectory(core)

if(NETWORK_ENABLED)
    add_subdirectory(network)
endif()
