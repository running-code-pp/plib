# plib

plib 是一个跨平台 C++ 库，提供多模块支持，包括多线程处理（线程池）、日志记录、配置解析、数据类型和工具函数等功能。项目采用模块化设计，旨在提供高性能、可扩展和易于维护的解决方案。 

> ✨ **亮点**  
> - 模块化设计  
> - 高性能并发支持  
> - 丰富的工具函数  
> - 易于扩展维护  

## 项目结构

```
d:\code\plib
├── CMakeLists.txt              # 根目录 CMake 配置，包含全局构建设置
├── core/                       # 核心模块（并发、配置、日志等）
│   ├── CMakeLists.txt          # 核心构建配置
│   ├── include/                # 头文件目录（包含: concurrent, config, log, type, utils 等）
│   └── src/                    # 源码实现，与 include 目录结构对应
├── framework/                  # 框架模块，扩展项目功能
│   └── CMakeLists.txt
├── network/                    # 网络模块，提供 HTTP 客户端及网络通讯功能
│   └── CMakeLists.txt
└── unittest/                  # 单元测试目录
    ├── CMakeLists.txt
    └── core_test.cpp
```

## 依赖项

项目通过 [Conan](https://conan.io/) 管理以下第三方依赖项：

- **zlib** (1.3)
- **nlohmann_json** (3.11.3)
- **spdlog** (1.13.0)
- **libcurl** (8.5.0)
- **asio** (1.29.0, header-only)
- **tinyxml2** (10.0.0)
- **yaml-cpp** (0.8.0)
- **gtest** (1.14.0)

## 构建说明

本项目采用 CMake 构建系统，并使用 Conan 进行依赖管理。请确保您的系统中已安装以下工具：

- CMake 3.1 或更高版本
- Conan 2.x
- Python 3.11 或更高版本

### 构建步骤

1. **准备 Conan 配置**  
   在项目根目录下执行以下命令，自动检测并配置默认 profile，并将 C++ 标准设置为 C++20：  
   ```bash
   conan profile detect --force
   conan profile update settings.compiler.cppstd=20 default
   ```

2. **安装依赖项**  
   在项目根目录下执行：  
   ```bash
   conan install . --output-folder=build --build=missing --settings=build_type=Debug
   ```  
   该命令将下载并构建缺失的依赖项，并生成 CMake 工具链文件。

3. **生成 CMake 工程**  
   进入 `build` 目录后执行：  
   ```bash
   cmake .. -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake
   ```  
   如有需要，可指定 Debug 或 Release 配置。

4. **编译项目**  
   在 `build` 目录下执行：  
   ```bash
   cmake --build .
   ```  
   编译完成后，将生成库文件和测试可执行文件。

5. **运行测试**  
   执行以下命令运行单元测试：  
   ```bash
   ctest
   ```  
   或直接运行生成的测试可执行文件（例如 `plib_tests`）。

## 进度表

| 模块               | 功能描述                                        | 进度        |
| ------------------ | ----------------------------------------------- | ----------- |
| ⚙️ 并发编程         | 线程池实现，支持普通模式、优先级及暂停模式，协程调度器         | ⚠️ 部分完成   |
| 📜 日志记录         | 多种日志输出模式和格式化支持                        | ✅ 已完成   |
| 🔧 配置解析         | YAML、JSON 等配置文件解析与管理                     | ✅ 已完成   |
| 🗃 数据类型与工具函数 | 常用数据结构、算法、字符串处理及路径操作              | ✅ 已完成   |
| 🌐 网络通讯         | HTTP/TCP/UDP/websocket 客户端/服务端及异步 I/O 功能（网络模块扩展中）         | ⚠️ 未开始 |
| 🚀 框架模块         | 上层应用构建扩展功能（待完善）                      | ⏳ 待完善   |
| 🧪 单元测试         | 各模块功能的全面测试                              | ⚠️ 部分完成   |

## 注意事项

- 请确保在启用 C++20 特性（例如 `/std:c++20`）的编译选项下构建项目。
- 单元测试使用 Google Test，相关测试代码位于 `unittest/` 目录下。
- 日志系统与配置解析模块的详细使用方法请参考源码和注释。

## 关于

如需更多信息，请参考项目源码中的文档和注释。若遇到问题或有改进建议，请通过项目 Issue 或直接联系项目维护者。  
欢迎一起共创更好的 plib！ 🚀