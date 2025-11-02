#include "utils/os_util.hpp"
#include "typedef.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <array>
#include <functional>
#include "utils/library_util.hpp"



// 跨平台头文件
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <tchar.h>
#include <RestartManager.h>
#pragma comment(lib, "advapi32.lib")
#else
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

namespace plib::core::utils
{
    // 定义 LOAD_SYMBOL 宏
#define LOAD_SYMBOL(lib, name) ::plib::core::utils::LoadMethod(lib, #name, name)
    namespace
    {
        // RSTRTMGR.DLL

        DWORD(__stdcall *RmStartSession)(
            _Out_ DWORD *pSessionHandle,
            _Reserved_ DWORD dwSessionFlags,
            _Out_writes_(CCH_RM_SESSION_KEY + 1) WCHAR strSessionKey[]);
        DWORD(__stdcall *RmRegisterResources)(
            _In_ DWORD dwSessionHandle,
            _In_ UINT nFiles,
            _In_reads_opt_(nFiles) LPCWSTR rgsFileNames[],
            _In_ UINT nApplications,
            _In_reads_opt_(nApplications) RM_UNIQUE_PROCESS rgApplications[],
            _In_ UINT nServices,
            _In_reads_opt_(nServices) LPCWSTR rgsServiceNames[]);
        DWORD(__stdcall *RmGetList)(
            _In_ DWORD dwSessionHandle,
            _Out_ UINT *pnProcInfoNeeded,
            _Inout_ UINT *pnProcInfo,
            _Inout_updates_opt_(*pnProcInfo) RM_PROCESS_INFO rgAffectedApps[],
            _Out_ LPDWORD lpdwRebootReasons);
        DWORD(__stdcall *RmShutdown)(
            _In_ DWORD dwSessionHandle,
            _In_ ULONG lActionFlags,
            _In_opt_ RM_WRITE_STATUS_CALLBACK fnStatus);
        DWORD(__stdcall *RmEndSession)(
            _In_ DWORD dwSessionHandle);

    } // namespace

    bool KillProcessByPid(p_pid_t pid)
    {
#ifdef _WIN32
        HANDLE process_handle = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pid);
        if (!process_handle)
        {
            const DWORD error = GetLastError();
            return error == ERROR_INVALID_PARAMETER || error == ERROR_NOT_FOUND;
        }

        const BOOL terminated = TerminateProcess(process_handle, static_cast<UINT>(-1));
        if (!terminated)
        {
            CloseHandle(process_handle);
            return false;
        }

        const DWORD wait_result = WaitForSingleObject(process_handle, 5000);
        CloseHandle(process_handle);
        return wait_result == WAIT_OBJECT_0;
#else
        int signal_to_send = SIGTERM;
        for (int attempt = 0; attempt < 100; ++attempt)
        {
            if (kill(pid, signal_to_send) == 0)
            {
                if (signal_to_send == SIGKILL)
                {
                    return true;
                }
            }
            else if (errno == ESRCH)
            {
                return true;
            }
            else if (errno == EPERM)
            {
                return false;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (attempt == 49)
            {
                signal_to_send = SIGKILL;
            }
        }
        return false;
#endif
    }

    bool CloseProcesses(const std::string &filename)
    {
#ifdef _WIN32
        static const auto loaded = [&]
        {
            const auto LibRstrtMgr = SafeLoadLibrary(L"rstrtmgr.dll");
            return LOAD_SYMBOL(LibRstrtMgr, RmStartSession) && 
            LOAD_SYMBOL(LibRstrtMgr, RmRegisterResources) &&
             LOAD_SYMBOL(LibRstrtMgr, RmGetList) &&
              LOAD_SYMBOL(LibRstrtMgr, RmShutdown) && 
              LOAD_SYMBOL(LibRstrtMgr, RmEndSession);
        }();
        if (!loaded)
        {
            return false;
        }

        auto session = DWORD();
        auto sessionKey = std::wstring(CCH_RM_SESSION_KEY + 1, wchar_t(0));
        auto error = RmStartSession(&session, 0, sessionKey.data());
        if (error != ERROR_SUCCESS)
        {
            return false;
        }
        const auto guard = TmpGuard([&]{ RmEndSession(session); });

        const auto path = std::wstring(filename.begin(), filename.end());
        auto nullterm = path.c_str();
        error = RmRegisterResources(
            session,
            1,
            &nullterm,
            0,
            nullptr,
            0,
            nullptr);
        if (error != ERROR_SUCCESS)
        {
            return false;
        }

        auto processInfoNeeded = UINT(0);
        auto processInfoCount = UINT(0);
        auto reason = DWORD();

        error = RmGetList(
            session,
            &processInfoNeeded,
            &processInfoCount,
            nullptr,
            &reason);
        if (error != ERROR_SUCCESS && error != ERROR_MORE_DATA)
        {
            return false;
        }
        else if (processInfoNeeded <= 0)
        {
            return true;
        }
        error = RmShutdown(session, RmForceShutdown, NULL);
        if (error != ERROR_SUCCESS)
        {
            return false;
        }
        return true;
#else
        // Linux / macOS 实现
        // 使用 lsof 查询占用文件的进程，并尝试 kill
        std::string command = "lsof \"" + filename + "\" | awk 'NR>1 {print $2}' | sort -u";
        FILE *pipe = popen(command.c_str(), "r");
        if (!pipe)
        {
            std::cerr << "Failed to run lsof\n";
            return false;
        }

        char pid_str[16];
        bool success = true;

        while (fgets(pid_str, sizeof(pid_str), pipe))
        {
            pid_t pid = std::stoi(std::string(pid_str));
            std::cout << "Trying to terminate process " << pid << " using file " << filename << "\n";

            if (kill(pid, SIGTERM) != 0)
            {
                std::cerr << "Failed to kill process " << pid << "\n";
                success = false;
            }
            else
            {
                // 可选：等待一下
                usleep(100000);
            }
        }

        pclose(pipe);
        return success;
#endif
    }

} // namespace plib::core::utils