/**
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-31 23:28:46
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-11-01 21:03:15
 * @FilePath: \plib\src\core\include\utils\file_lock.hpp
 * @Description: file lock的跨平台实现
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
#ifndef PLIB_CORE_UTILS_FILE_LOCK_HPP_
#define PLIB_CORE_UTILS_FILE_LOCK_HPP_

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#else
// posix 标准
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>
#endif
#include <memory>

namespace plib::core::utils
{
    class FileLock
    {
    public:
        FileLock();
        ~FileLock();

        bool lock(const char* file_path, const char* mode);
        void unlock();
        [[nodiscard]] bool is_locked() const;
        static constexpr auto kSkipBytes = std::size_t{4};
        void setFile(FILE* file);
    private:
        class Lock;
        static constexpr auto kLockOffset = std::size_t{0};
        static constexpr auto kLockLimit = kSkipBytes;
        FILE* _file;
        std::unique_ptr<Lock> _lock;
    };

} // namespace plib::core::utils

#endif