/**
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-11-01 21:05:15
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-11-01 21:05:38
 * @FilePath: \plib\src\core\include\typedef.hpp
 * @Description: 类型别名，主要是为了跨平台考虑, p_xxx_t 表示plib中的类型别名
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
#ifndef PLIB_CORE_TYPEDEF_HPP_
#define PLIB_CORE_TYPEDEF_HPP_

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace plib::core
{

    // 文件描述符/windows句柄
#ifdef _WIN32
    using p_descriptor_t = HANDLE;

#else
    using p_descriptor_t = int;
#endif

// 进程id
#ifdef _WIN32
    using p_pid_t = DWORD;
#else
    using p_pid_t = pid_t;

#endif

}

#endif // PLIB_CORE_TYPEDEF_HPP_