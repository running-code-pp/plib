/**
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-11-02 00:10:53
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-11-02 00:24:48
 * @FilePath: \plib\src\core\include\crashreport\crash_report_writer.hpp
 * @Description:  崩溃报告记录器,使用breakpad
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
#ifndef PLIB_CORE_CRASHREPORT_WRITER_HPP_
#define PLIB_CORE_CRASHREPORT_WRITER_HPP_

#include <string>
#include "utils/file_lock.hpp"
#include <stdio.h>
#include <optional>
#include <vector>
#include <map>
#include "concurrent/spinlock.hpp"
#include "type/singleton.hpp"
#include <atomic>
#include <memory>
#include <sstream>

// 前向声明 Breakpad 类型
#if defined(_WIN32)
namespace google_breakpad
{
    class ExceptionHandler;
}
#elif defined(__linux__)
namespace google_breakpad
{
    class ExceptionHandler;
    class MinidumpDescriptor;
}
#elif defined(__APPLE__)
namespace google_breakpad
{
    class ExceptionHandler;
}
#endif

namespace plib::core::crashreport
{

    class CrashReportWriter : public plib::core::type::Singleton<CrashReportWriter>
    {
    public:
        friend class plib::core::type::Singleton<CrashReportWriter>;
        void start();
        void addAnnotation(const std::string &key, const std::string &value);
        void setReportPath(const std::string &path);
        std::ostringstream &getHeaderStream();
        void flushHeaderToReport();

    private:
        CrashReportWriter();
        ~CrashReportWriter();

    private:
        /**
         * @brief 打开报告文件，准备写入
         * @return true 成功打开,false 失败
         */
        bool openReport();

        /**
         * @brief 关闭报告文件
         */
        void closeReport();

        /**
         * @brief 开始捕获崩溃,设置崩溃处理器，准备捕获系统崩溃信号
         */
        void startCatching();

        /**
         * @brief 结束崩溃捕获(清理相关资源)
         */
        void finishCatching();

        [[nodiscard]] std::string reportPath() const;
        [[nodiscard]] std::map<std::string, std::string> annotations() const;

        std::atomic_bool _isOpen;
        std::atomic_bool _isInitial;
        plib::core::concurrent::Spinlock _lock;
        std::string _reportPath;
        plib::core::utils::FileLock _fileLock;
        FILE *_reportFile;
        std::map<std::string, std::string> _annotations;

        // Breakpad 异常处理器
        std::unique_ptr<google_breakpad::ExceptionHandler> _exceptionHandler;

        // 储存头部信息
        std::ostringstream _headerStream;
    };

} // namespace plib::core::crashreport

#endif // PLIB_CORE_CRASHREPORT_WRITER_HPP_