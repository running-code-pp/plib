#include "crashreport/crash_report_writer.hpp"

#if defined(_WIN32)
#include <client/windows/handler/exception_handler.h>
#elif defined(__linux__)
#include <client/linux/handler/exception_handler.h>
#elif defined(__APPLE__)
#include <client/mac/handler/exception_handler.h>
#endif
#include <mutex>
#include "utils/platform_info.hpp"
#include <cassert>
#include "utils/time_util.hpp"
#include <filesystem>
#include <iostream>

#define PREALLOC_HEADER_SIZE (1024 * 1024) // 预分配头部信息大小 1MB

namespace plib::core::crashreport
{

    bool is_skip_write_header = false; // 是否跳过写入头部信息
    bool is_header_written = false;    // 头部信息是否已写入
    std::mutex _mtx;                   // 互斥写入

#if defined(_WIN32)
    // Windows 回调函数必须是静态的或全局的
    static bool MinidumpCallback(
        const wchar_t *dump_path,
        const wchar_t *minidump_id,
        void *context,
        EXCEPTION_POINTERS *exinfo,
        MDRawAssertionInfo *assertion,
        bool succeeded)
    {
        CrashReportWriter::GetInstance()->flushHeaderToReport();
        if (succeeded)
        {
            wprintf(L"\n=== CRASH DETECTED ===\n");
            wprintf(L"Minidump created successfully!\n");
            wprintf(L"Location: %s\\%s.dmp\n", dump_path, minidump_id);
            wprintf(L"======================\n");
            fflush(stdout);
        }
        else
        {
            wprintf(L"\nFailed to create minidump!\n");
            fflush(stdout);
        }
        return succeeded;
    }
#elif defined(__linux__)
    static bool MinidumpCallback(
        const google_breakpad::MinidumpDescriptor &descriptor,
        void *context,
        bool succeeded)
    {
        if (succeeded)
        {
            printf("\n=== CRASH DETECTED ===\n");
            printf("Minidump: %s\n", descriptor.path());
            printf("======================\n");
            fflush(stdout);
        }
        return succeeded;
    }
#elif defined(__APPLE__)
    static bool MinidumpCallback(
        const char *dump_dir,
        const char *minidump_id,
        void *context,
        bool succeeded)
    {
        if (succeeded)
        {
            printf("\n=== CRASH DETECTED ===\n");
            printf("Minidump: %s/%s.dmp\n", dump_dir, minidump_id);
            printf("======================\n");
            fflush(stdout);
        }
        return succeeded;
    }
#endif

    CrashReportWriter::CrashReportWriter() : _isInitial(false), _isOpen(false), _reportPath(""), _reportFile(nullptr)
    {
    }

    CrashReportWriter::~CrashReportWriter()
    {
        finishCatching();
        closeReport();
    }

    void CrashReportWriter::flushHeaderToReport()
    {
        if (!is_header_written)
        {
            std::lock_guard<std::mutex> lock(_mtx);
            if (!is_header_written)
            {
                for (const auto &[key, value] : _annotations)
                {
                    _headerStream << key << ": " << value << "\n";
                }
                // 一次性写入report文件
                if (_reportFile)
                {
                    fwrite(_headerStream.str().c_str(), 1, _headerStream.str().size(), _reportFile);
                }
                fclose(_reportFile);
                _reportFile = nullptr;
            }
        }

        is_header_written = true;
    }

    std::ostringstream &CrashReportWriter::getHeaderStream()
    {
        return _headerStream;
    }

    void CrashReportWriter::setReportPath(const std::string &path)
    {
        _reportPath = path;
        // 确保传入的path是dir
        assert(!_reportPath.empty() && (_reportPath.back() == '/' || _reportPath.back() == '\\'));
        _isInitial = true;
        _reportPath = path;
    }

    void CrashReportWriter::start()
    {
        addAnnotation("Launched", plib::core::utils::getNowTimeFormatStr("%Y-%m-%d %H:%M:%S"));
        addAnnotation("Platform", plib::core::utils::PlatformString());
        // 创建报告所在目录
        std::filesystem::create_directories(_reportPath);
        openReport();
        startCatching();
    }

    void CrashReportWriter::addAnnotation(const std::string &key, const std::string &value)
    {
        _lock.lock();
        _annotations.emplace(key, value);
        _lock.unlock();
    }

    // 自定义崩溃信息记录的文件路径
    std::string CrashReportWriter::reportPath() const
    {
        return (std::filesystem::absolute(_reportPath) / "report.txt").string();
    }

    std::map<std::string, std::string> CrashReportWriter::annotations() const
    {
        return _annotations;
    }

    bool CrashReportWriter::openReport()
    {
        if (_isOpen)
        {
            return true;
        }
        _reportFile = fopen(reportPath().c_str(), "w+");
        if (!_reportFile)
        {
            std::cerr << "Failed to open report file: " << reportPath() << std::endl;
            return false;
        }
        _fileLock.setFile(_reportFile);
        if (!_fileLock.lock(reportPath().c_str(), "w+"))
        {
            fclose(_reportFile);
            _reportFile = nullptr;
            return false;
        }

        _isOpen = true;
        return true;
    }

    void CrashReportWriter::closeReport()
    {
        _isOpen = false;
        if (_reportFile)
        {
            flushHeaderToReport();
            fclose(_reportFile);
            _reportFile = nullptr;
        }
    }

    void CrashReportWriter::startCatching()
    {
        std::cout << "Starting crash handler..." << std::endl;

        // 转换为绝对路径
        auto abs_path = std::filesystem::absolute(_reportPath);
        std::string abs_path_str = abs_path.string();

        std::cout << "Absolute dump path: " << abs_path_str << std::endl;

        // 安装breakpad崩溃处理器
#if defined(_WIN32)
        // Windows: 使用宽字符路径
        std::wstring wpath = abs_path.wstring();

        wprintf(L"Dump directory (wide): %s\n", wpath.c_str());

        _exceptionHandler = std::make_unique<google_breakpad::ExceptionHandler>(
            wpath,
            nullptr,          // filter callback
            MinidumpCallback, // 使用静态函数而不是 lambda
            nullptr,          // callback_context
            google_breakpad::ExceptionHandler::HANDLER_ALL,
            MiniDumpNormal,   // 转储类型
            (HANDLE) nullptr, // pipe_handle
            nullptr           // crash_generation_client
        );

        if (_exceptionHandler)
        {
            std::cout << "✓ Breakpad handler installed successfully!" << std::endl;
        }
        else
        {
            std::cerr << "✗ FAILED to install Breakpad handler!" << std::endl;
        }

#elif defined(__linux__)
        // Linux: 使用 MinidumpDescriptor
        google_breakpad::MinidumpDescriptor descriptor(_reportPath);
        _exceptionHandler = std::make_unique<google_breakpad::ExceptionHandler>(
            descriptor,
            nullptr,          // filter callback
            MinidumpCallback, // 使用静态函数
            nullptr,          // callback_context
            true,             // install_handler (安装信号处理器)
            -1                // server_fd
        );

#elif defined(__APPLE__)
        // macOS: 使用字符串路径
        _exceptionHandler = std::make_unique<google_breakpad::ExceptionHandler>(
            _reportPath,
            nullptr,          // filter callback
            MinidumpCallback, // 使用静态函数
            nullptr,          // callback_context
            true,             // install_handler
            nullptr           // port_name
        );

#else
        std::cerr << "Platform not supported for Breakpad crash reporting!" << std::endl;
#endif
    }

    void CrashReportWriter::finishCatching()
    {
        _exceptionHandler.reset();
        _exceptionHandler = nullptr;
    }
}