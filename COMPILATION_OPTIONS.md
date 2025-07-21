# 编译选项说明

## 可用的编译选项

### CORE_CONFIG
- **默认值**: ON
- **功能**: 启用CORE/CONFIG模块
- **影响**: 包含`config/`目录下的所有源文件和头文件
- **编译定义**: `CORE_CONFIG_ENABLED`

### CORE_TYPE  
- **默认值**: ON
- **功能**: 启用CORE/TYPE模块
- **影响**: 包含`type/`目录下的所有源文件和头文件
- **编译定义**: `CORE_TYPE_ENABLED`

## 使用方法

### 1. 命令行方式

```bash
# 启用所有模块
cmake -B build -DCORE_CONFIG=ON -DCORE_TYPE=ON

# 禁用CONFIG模块
cmake -B build -DCORE_CONFIG=OFF -DCORE_TYPE=ON

# 禁用TYPE模块
cmake -B build -DCORE_CONFIG=ON -DCORE_TYPE=OFF

# 禁用所有模块
cmake -B build -DCORE_CONFIG=OFF -DCORE_TYPE=OFF
```

### 2. CMake GUI方式

在CMake GUI中，你可以看到并修改这些选项：
- `CORE_CONFIG` - 复选框
- `CORE_TYPE` - 复选框

### 3. Visual Studio方式

在Visual Studio中，右键项目 -> 属性 -> CMake -> CMake设置，可以修改这些选项。

## 目录结构

```
src/core/
├── CMakeLists.txt           # 模块构建配置
├── core.cpp                 # 主源文件
├── config/                  # CONFIG模块 (可选)
│   ├── memory/
│   │   ├── memory.h
│   │   └── memory.cpp
│   └── cache/
│       ├── redis.h
│       └── redis.cpp
├── type/                    # TYPE模块 (可选)
│   ├── types.h
│   └── types.cpp
└── private_inc/             # 私有头文件目录
    ├── internal.h
    └── utils.h
```

## 条件编译

在代码中可以使用编译定义进行条件编译：

```cpp
#ifdef CORE_CONFIG_ENABLED
    #include "config/memory/memory.h"
    // CONFIG模块相关代码
#endif

#ifdef CORE_TYPE_ENABLED
    #include "type/types.h"
    // TYPE模块相关代码
#endif
```

## Visual Studio文件组织

启用模块后，文件会在Visual Studio中按以下方式组织：

- **Source Files** - 所有源文件
- **Source Files\Config** - CONFIG模块源文件 (如果启用)
- **Source Files\Type** - TYPE模块源文件 (如果启用)
- **Header Files** - 所有头文件
- **Header Files\Config** - CONFIG模块头文件 (如果启用)
- **Header Files\Type** - TYPE模块头文件 (如果启用)

## 构建输出

构建时会显示模块状态：

```
-- CORE_CONFIG module enabled
-- CORE_TYPE module enabled
-- Core config source files: /path/to/config/memory.cpp
-- Core type source files: /path/to/type/types.cpp
```

## 依赖关系

- **CORE_CONFIG**: 依赖 spdlog, nlohmann_json, tinyxml2
- **CORE_TYPE**: 依赖 spdlog, nlohmann_json, tinyxml2
- 两个模块都会包含在`plib_core`动态库中

## 示例

### 启用所有模块
```bash
cmake -B build -DCORE_CONFIG=ON -DCORE_TYPE=ON
cmake --build build --config Release
```

### 只启用CONFIG模块
```bash
cmake -B build -DCORE_CONFIG=ON -DCORE_TYPE=OFF
cmake --build build --config Release
```

### 只启用TYPE模块
```bash
cmake -B build -DCORE_CONFIG=OFF -DCORE_TYPE=ON
cmake --build build --config Release
``` 