# plib 项目结构

## 模块化设计

plib采用模块化设计，分为三个主要模块：

### 1. Core 模块 (plib_core)
- **位置**: `modules/core/`
- **公共头文件**: `include/core/`
- **私有头文件**: `modules/core/private_inc/`
- **依赖**: spdlog, nlohmann_json
- **功能**: 核心功能，日志，配置等

### 2. Net 模块 (plib_net)
- **位置**: `modules/net/`
- **公共头文件**: `include/net/`
- **私有头文件**: `modules/net/private_inc/`
- **依赖**: plib_core, libcurl, asio, spdlog
- **功能**: 网络功能，HTTP客户端，异步IO等

### 3. Framework 模块 (plib_framework)
- **位置**: `modules/framework/`
- **公共头文件**: `include/framework/`
- **私有头文件**: `modules/framework/private_inc/`
- **依赖**: plib_core, plib_net, cryptopp, spdlog, nlohmann_json
- **功能**: 高级框架功能，加密，序列化等

## 目录结构

```
plib/
├── CMakeLists.txt              # 主CMake配置
├── conanfile.txt               # Conan依赖
├── include/                    # 公共头文件
│   ├── plib.hpp               # 主头文件
│   ├── core/                  # Core模块公共头文件
│   ├── net/                   # Net模块公共头文件
│   └── framework/             # Framework模块公共头文件
├── modules/                    # 模块源码
│   ├── core/                  # Core模块
│   │   ├── CMakeLists.txt
│   │   ├── core.cpp
│   │   └── private_inc/      # 私有头文件
│   ├── net/                   # Net模块
│   │   ├── CMakeLists.txt
│   │   ├── net.cpp
│   │   └── private_inc/      # 私有头文件
│   └── framework/             # Framework模块
│       ├── CMakeLists.txt
│       ├── framework.cpp
│       └── private_inc/      # 私有头文件
├── examples/                   # 示例程序
├── unittest/                   # 单元测试
└── build/                      # 构建输出
```

## 命名空间结构

```cpp
namespace plib {
    namespace core {
        // Core模块功能
    }
    
    namespace net {
        // Net模块功能
    }
    
    namespace framework {
        // Framework模块功能
    }
}
```

## 构建输出

每个模块都会生成独立的动态库：
- `plib_core.dll` / `libplib_core.so`
- `plib_net.dll` / `libplib_net.so`
- `plib_framework.dll` / `libplib_framework.so`

## 使用方式

```cpp
#include <plib/plib.hpp>

int main() {
    // 使用core模块
    auto core_version = plib::core::get_core_version();
    
    // 使用net模块
    auto net_version = plib::net::get_net_version();
    
    // 使用framework模块
    auto framework_version = plib::framework::get_framework_version();
    
    return 0;
}
``` 