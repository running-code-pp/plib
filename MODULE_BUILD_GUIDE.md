# 模块构建指南

## 递归扫描功能

新的CMakeLists.txt支持递归扫描所有子目录的源文件和头文件：

### 支持的源文件格式
- `*.cpp` - C++源文件
- `*.c` - C源文件  
- `*.cc` - C++源文件
- `*.cxx` - C++源文件

### 支持的头文件格式
- `*.h` - C/C++头文件
- `*.hpp` - C++头文件
- `*.hh` - C++头文件
- `*.hxx` - C++头文件

## 目录结构

```
src/core/
├── CMakeLists.txt           # 模块构建配置
├── core.cpp                 # 主源文件
├── config/                  # 配置子目录
│   ├── memory/
│   │   ├── memory.h
│   │   └── memory.cpp
│   └── cache/
│       ├── redis.h
│       └── redis.cpp
├── log/                     # 日志子目录
│   ├── logger.h
│   └── logger.cpp
└── private_inc/             # 私有头文件目录
    ├── internal.h
    └── utils.h
```

## 自动发现功能

CMakeLists.txt会自动发现：

1. **所有源文件** - 递归扫描所有子目录
2. **所有头文件** - 递归扫描所有子目录
3. **公共头文件** - `include/core/` 目录下的头文件
4. **私有头文件** - `private_inc/` 目录下的头文件

## Visual Studio 文件组织

文件会在Visual Studio中按以下方式组织：

- **Source Files** - 所有源文件
- **Header Files** - 所有头文件
- **Header Files\Public** - 公共头文件
- **Header Files\Private** - 私有头文件

## 依赖管理

每个模块可以独立配置依赖：

```cmake
# Core模块依赖
target_link_libraries(plib_core
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    tinyxml2::tinyxml2
)

# Net模块依赖
target_link_libraries(plib_net
    plib_core
    CURL::libcurl
    asio::asio
    spdlog::spdlog
)

# Framework模块依赖
target_link_libraries(plib_framework
    plib_core
    plib_net
    cryptopp::cryptopp
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)
```

## 使用模板

要创建新模块，可以：

1. 复制 `CMakeLists_template.txt`
2. 将 `MODULE_NAME` 替换为你的模块名
3. 根据需要调整依赖项
4. 在主CMakeLists.txt中添加 `add_subdirectory(modules/your_module)`

## 调试信息

构建时会显示发现的文件：

```
-- Core module source files: /path/to/core.cpp;/path/to/config/memory.cpp
-- Core module header files: /path/to/core.h;/path/to/config/memory.h
-- Core module public headers: /path/to/include/core/core.hpp
-- Core module private headers: /path/to/private_inc/internal.h
``` 