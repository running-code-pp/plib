#ifndef PLIB_CORE_UTILS_PLATFORM_INFO_HPP_
#define PLIB_CORE_UTILS_PLATFORM_INFO_HPP_

namespace plib::core::utils
{
    // ============ 处理器架构判断 ============

    // 判断是否为 x86 32 位架构
    inline constexpr bool IsX86_32()
    {
#if defined(__i386__) || defined(_M_IX86) || defined(__X86__) || defined(_X86_)
        return true;
#else
        return false;
#endif
    }

    // 判断是否为 x86 64 位架构
    inline constexpr bool IsX86_64()
    {
#if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__) || defined(_M_AMD64)
        return true;
#else
        return false;
#endif
    }

    // 判断是否为 ARM 64 位架构
    inline constexpr bool IsARM64()
    {
#if defined(__aarch64__) || defined(_M_ARM64) || defined(__arm64__)
        return true;
#else
        return false;
#endif
    }

    // ============ 操作系统判断 ============

    // 判断是否为 Windows 系统
    inline constexpr bool IsWindows()
    {
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
        return true;
#else
        return false;
#endif
    }

    // 判断是否为 Windows 32 位
    inline constexpr bool IsWindows32Bit()
    {
#if (defined(_WIN32) || defined(__WIN32__)) && !defined(_WIN64)
        return true;
#else
        return false;
#endif
    }

    // 判断是否为 Windows 64 位
    inline constexpr bool IsWindows64Bit()
    {
#if defined(_WIN64)
        return true;
#else
        return false;
#endif
    }

    // 判断是否为 Windows Store 应用构建
    inline constexpr bool IsWindowsStoreBuild()
    {
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP || WINAPI_FAMILY == WINAPI_FAMILY_PC_APP)
        return true;
#elif defined(OS_WIN_STORE) // 自定义宏
        return true;
#else
        return false;
#endif
    }

    // 判断是否为 macOS 系统
    inline constexpr bool IsMac()
    {
#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#if TARGET_OS_MAC && !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
        return true;
#else
        return false;
#endif
#else
        return false;
#endif
    }

    // 判断是否为 Mac App Store 构建
    inline constexpr bool IsMacStoreBuild()
    {
#if defined(OS_MAC_STORE) // 需要在构建时定义此宏
        return true;
#else
        return false;
#endif
    }

    // 判断是否为 Linux 系统
    inline constexpr bool IsLinux()
    {
#if defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
        return true;
#else
        return false;
#endif
    }

    // 判断是否为 Unix-like 系统
    inline constexpr bool IsUnix()
    {
#if defined(__unix__) || defined(__unix) || defined(unix)
        return true;
#else
        return false;
#endif
    }

    // 判断是否为 POSIX 兼容系统
    inline constexpr bool IsPosix()
    {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
        return true;
#else
        return false;
#endif
    }

    // ============ 编译器判断 ============

    // 判断是否使用 MSVC 编译器
    inline constexpr bool IsMSVC()
    {
#if defined(_MSC_VER)
        return true;
#else
        return false;
#endif
    }

    // 判断是否使用 GCC 编译器
    inline constexpr bool IsGCC()
    {
#if defined(__GNUC__) && !defined(__clang__)
        return true;
#else
        return false;
#endif
    }

    // 判断是否使用 Clang 编译器
    inline constexpr bool IsClang()
    {
#if defined(__clang__)
        return true;
#else
        return false;
#endif
    }

    // ============ 平台字符串 ============

    // 获取平台标识字符串
    inline constexpr const char *PlatformString()
    {
        if constexpr (IsWindowsStoreBuild())
        {
            return IsWindows64Bit() ? "WinStore64Bit" : "WinStore32Bit";
        }
        else if constexpr (IsWindows32Bit())
        {
            return "Windows32Bit";
        }
        else if constexpr (IsWindows64Bit())
        {
            return "Windows64Bit";
        }
        else if constexpr (IsMacStoreBuild())
        {
            return "MacAppStore";
        }
        else if constexpr (IsMac())
        {
            return "MacOS";
        }
        else if constexpr (IsLinux())
        {
            return "Linux";
        }
        else
        {
            return "Unknown";
        }
    }

} // namespace plib::core::utils

#endif // PLIB_CORE_UTILS_PLATFORM_INFO_HPP_