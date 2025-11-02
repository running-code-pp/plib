/**
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-11-02 00:10:38
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-11-02 01:27:59
 * @FilePath: \plib\src\core\include\crashreport\crash_report.hpp
 * @Description:
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
#ifndef PLIB_CORE_CRASHREPORT_HPP_
#define PLIB_CORE_CRASHREPORT_HPP_

#include <string>
#include <cstring>
#include "crashreport/crash_report_writer.hpp"

namespace plib::core::crashreport
{
    struct CrashReport
    {
    };

    template <typename T>
    CrashReport operator<<(CrashReport, const T &value)
    {
        CrashReportWriter::GetInstance().getHeaderStream() << value;
        return CrashReport();
    }

    // 重载处理宽字符串（优先级高于模板）
    template <typename T = std::wstring>
    CrashReport operator<<(CrashReport, const std::wstring &value)
    {
        CrashReportWriter::GetInstance().getHeaderStream() << std::string(value.begin(), value.end());
        return CrashReport();
    }

    template <typename T = const wchar_t *>
    CrashReport operator<<(CrashReport, const wchar_t *value)
    {
        CrashReportWriter::GetInstance().getHeaderStream() << std::string(value, value + wcslen(value));
        return CrashReport();
    }
} // namespace plib::core::crashreport

#endif