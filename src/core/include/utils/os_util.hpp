/**
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-11-01 20:46:06
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-11-01 21:52:23
 * @FilePath: \plib\src\core\include\utils\os_util.hpp
 * @Description:
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
/**
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-11-01 20:46:06
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-11-01 20:58:23
 * @FilePath: \plib\src\core\include\utils\os_util.hpp
 * @Description: 系统工具
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
#ifndef PLIB_CORE_UTILS_OS_UTIL_HPP_
#define PLIB_CORE_UTILS_OS_UTIL_HPP_

#include "typedef.hpp"
#include <string>
#include <iostream>
#include <functional>
#ifdef _WIN32
#include <windows.h>
#endif

namespace plib::core::utils
{

        // TODO: 移动到utils目录中单独一个tool.hpp中并完善
    struct TmpGuard
    {
        TmpGuard(const std::function<void()> &deleter) : deleter_(deleter) {}
        ~TmpGuard()
        {
            deleter_();
        }

    private:
        std::function<void()> deleter_;
    };

    // 根据pid 终止进程
    bool KillProcessByPid(p_pid_t pid);

    // 终止占用某个文件的进程
    bool CloseProcesses(const std::string &filename);

} // namespace plib::core::utils

#endif // PLIB_CORE_UTILS_OS_UTIL_HPP_