 # plib - Cross-platform C++ Utility Library

A modern cross-platform C++ utility library built with Conan2 and CMake.

## Supported Platforms

- **Windows**: Windows 10/11 (x64)
- **Linux**: Ubuntu 18.04+, CentOS 7+, and other distributions
- **macOS**: macOS 10.15+ (x64 and Apple Silicon)

## Dependencies

- zlib/1.3
- nlohmann_json/3.11.3
- spdlog/1.13.0
- cryptopp/8.9.0
- libcurl/8.5.0
- asio/1.29.0

## Building

### Prerequisites

- CMake 3.20 or higher
- Conan 2.x
- C++17 compatible compiler
  - Windows: Visual Studio 2019+ or MSVC
  - Linux: GCC 7+ or Clang 6+
  - macOS: Xcode 11+ or Clang

### Quick Build

#### Windows
```cmd
install.bat
```

#### Linux/macOS
```bash
chmod +x install.sh
./install.sh
```

#### Python Script (Cross-platform)
```bash
python build_cross_platform.py
```

### Manual Build

```bash
# Create build directory
mkdir build && cd build

# Install dependencies
conan install .. --build=missing

# Configure project
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release

# Build project
cmake --build . --config Release
```

## Project Structure

```
plib/
├── CMakeLists.txt              # Main CMake configuration
├── conanfile.txt               # Conan dependencies
├── build_cross_platform.py     # Cross-platform Python build script
├── install.bat                 # Windows installation script
├── install.sh                  # Linux/macOS installation script
├── include/                    # Header files
│   └── plib/
│       ├── config.hpp          # Platform-specific configurations
│       └── plib.hpp           # Main library header
├── src/                        # Source files
│   └── CMakeLists.txt
├── examples/                   # Example programs
│   └── CMakeLists.txt
└── tests/                      # Test files
    └── CMakeLists.txt
```

## Cross-platform Features

### Platform Detection
The library automatically detects the platform and compiler:
- Windows (MSVC, MinGW)
- Linux (GCC, Clang)
- macOS (Clang)

### Dynamic Library Support
- **Windows**: DLL export/import with `__declspec`
- **Linux/macOS**: Symbol visibility with `__attribute__`

### Compiler Optimizations
- Platform-specific compiler flags
- Cross-platform inline and force-inline macros
- Thread-local storage support
- Branch prediction hints

## Usage

```cpp
#include <plib/plib.hpp>

int main() {
    // Get library version
    const char* version = plib::get_version();
    
    // Platform detection
    #ifdef PLIB_WINDOWS
        // Windows-specific code
    #elif defined(PLIB_LINUX)
        // Linux-specific code
    #elif defined(PLIB_MACOS)
        // macOS-specific code
    #endif
    
    return 0;
}
```

## License

See LICENSE file for details.