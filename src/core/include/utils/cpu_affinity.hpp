/** 
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-22 14:11:14
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-22 14:12:21
 * @FilePath: \plib\src\core\include\utils\cpu_affinity.hpp
 * @Description:设置线程cpu亲和性
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
#ifndef PLIB_CORE_UTILS_CPU_AFFINITY_HPP_
#define PLIB_CORE_UTILS_CPU_AFFINITY_HPP_
#include <thread>
#include <vector>
#include <fstream>
#ifdef _WIN32
#include <windows.h>
#include <intrin.h>  // for __cpuid
#else
#include <pthread.h>
#include <sched.h>
#include <unistd.h>  // for sysconf
#include <cpuid.h>   // for __get_cpuid
#endif

/**
 * @brief: 检测是否开启了超线程
 * @return: true:开启了,false:未开启
 */
 
bool is_hyper_threading_enabled() {
  #ifdef _WIN32
    DWORD len = 0;
    GetLogicalProcessorInformation(nullptr, &len);
    std::vector<BYTE> buffer(len);
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(buffer.data());
    
    if (!GetLogicalProcessorInformation(info, &len)) {
        return false;
    }
    
    int physical_cores = 0,logical_cores=0;
    while(true){
        if(info->Relationship == RelationProcessorCore){
            physical_cores++;
            // calculate the number of core
            DWORD_PTR mask = info->ProcessorMask;
            while (mask) {
                logical_cores += (mask & 1);
                mask >>= 1;
            }
        }
    }
    return logical_cores > physical_cores;
  #else
    std::ifstream cpuinfo("/proc/cpuinfo");
    if(!cpuinfo.is_open()){
        return false;
    }
    std::string line;
    int physical_cores = 0, logical_cores = 0;
    while(std::getline(cpuinfo, line)){
        if(line.find("cpu cores") != std::string::npos){
            physical_cores = std::stoi(line.substr(line.find(":") + 1));
        }else if(line.find("siblings") != std::string::npos){
            logical_cores = std::stoi(line.substr(line.find(":") + 1));
        }
        if(physical_cores && logical_cores){
            break;
        }
    }
    file.close();
    if(physical_cores == 0 || logical_cores == 0){
        return false;
    }
    return logical_cores > physical_cores;
  #endif
}


/**
 * @brief: 获取当前线程的句柄
 * @return: 返回当前线程的句柄
 */
std::thread::native_handle_type get_current_thread_handle() {
#ifdef _WIN32
    return GetCurrentThread();
#else 
    return pthread_self();
#endif
}

/**
 * 将当前进程绑定到指定的CPU核心。
 * @param core_id 要绑定到的CPU核心ID（从0开始）。
 * @return 如果成功返回true，失败返回false。
 */
bool set_process_affinity(int core_id) {
#ifdef _WIN32
    // Windows 实现
    // 创建一个掩码，只允许在指定的核心上运行
    DWORD_PTR process_mask = 1ULL << core_id;
    
    // 获取当前进程的句柄
    HANDLE hProcess = GetCurrentProcess();
    
    // 设置进程的亲和性掩码
    BOOL result = SetProcessAffinityMask(hProcess, process_mask);
    
    // SetProcessAffinityMask 在成功时返回非零值
    return result != 0;
#else
    // Linux/Unix (POSIX) 实现
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);           // 清空CPU集合
    CPU_SET(core_id, &cpuset);   // 将指定的核心ID添加到集合中

    // 获取当前进程的ID
    pid_t pid = 0; // 传递0表示当前进程
    // pthread_setaffinity_np 用于线程，但传递0作为pid可以影响主线程（即进程）
    // 更标准的进程绑定使用 sched_setaffinity
    int result = sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
    
    // sched_setaffinity 在成功时返回0
    return result == 0;
#endif
}

/**
 * 获取当前进程的CPU亲和性掩码中最低有效位（即通常首选的核心），
 * 或者在Linux下尝试获取主线程的当前CPU（如果进程只有一个线程）。
 * 注意：进程可以绑定到多个核心，此函数返回其中一个（通常是集合中的第一个）。
 * @return 成功时返回一个CPU核心ID (>=0)，失败时返回 -1。
 */
int get_current_process_cpu_id() {
#ifdef _WIN32
    // Windows 实现: 获取进程的亲和性掩码
    HANDLE hProcess = GetCurrentProcess();
    DWORD_PTR processAffinityMask, systemAffinityMask;

    if (!GetProcessAffinityMask(hProcess, &processAffinityMask, &systemAffinityMask)) {
        return -1; // 获取失败
    }

    // 如果掩码为0，表示没有设置或错误
    if (processAffinityMask == 0) {
        return -1;
    }

    // 找到掩码中最低位的1，这通常是操作系统首选的核心
    unsigned long index;
    // 使用 BitScanForward 找到第一个置位的位
    if (_BitScanForward(&index, static_cast<unsigned long>(processAffinityMask))) {
        return static_cast<int>(index);
    }
    // 如果低位32位没有找到，检查高位
    if (_BitScanForward(&index, static_cast<unsigned long>(processAffinityMask >> 32))) {
        return static_cast<int>(index + 32);
    }
    return -1; // 理论上不会到这里，因为掩码非零

#else
    // Linux/Unix 实现
    // 方法1: 获取进程的CPU亲和性，返回集合中的第一个CPU
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    pid_t pid = 0; // 当前进程

    if (sched_getaffinity(pid, sizeof(cpuset), &cpuset) != 0) {
        return -1; // 获取失败
    }

    // 遍历CPU集合，找到第一个被设置的核心
    int num_cores = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
    for (int i = 0; i < num_cores; ++i) {
        if (CPU_ISSET(i, &cpuset)) {
            return i; // 返回亲和性掩码中的第一个核心
        }
    }
    return -1; // 未找到，理论上不会发生

    // 方法2 (可选): 返回当前主线程正在运行的核心
    // 这在单线程进程中很有意义，但在多线程中可能不代表整个进程。
    // return sched_getcpu(); 
#endif
}


void set_thread_affinity(std::thread::native_handle_type thread_handle, int core_id) {
#ifdef _WIN32
  // Windows实现
  DWORD_PTR mask = 1ULL << core_id;
  SetThreadAffinityMask(thread_handle, mask);
#else
  // Linux/Unix实现
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);
  pthread_setaffinity_np(thread_handle, sizeof(cpu_set_t), &cpuset);
#endif
}

/**
 * 获取当前线程正在运行的CPU核心ID。
 * 注意：此函数返回的是调用时线程被调度到的具体核心。
 * @return 成功时返回CPU核心ID (>=0)，失败时返回 -1。
 */
int get_thread_cpu_id(std::thread::native_handle_type thread_handle) {
#ifdef _WIN32
    // Windows 实现: 使用 GetNativeSystemInfo 和 GetThreadGroupAffinity
    // 但更简单且常用的方法是使用 GetLogicalProcessorInformation 或直接尝试获取，
    // 不过最直接反映"当前运行在哪个核"的是 GetThreadIdealProcessorEx（如果可用）或轮询。
    // 此处提供一个基于 GetThreadIdealProcessorEx 的实现（Windows 7 SP1+）。
    
    
    GROUP_AFFINITY groupAffinity;
    ZeroMemory(&groupAffinity, sizeof(groupAffinity));
    
    // 获取线程的组亲和性（对于单NUMA节点系统，通常是GROUP 0）
    if (!GetThreadGroupAffinity(thread_handle, &groupAffinity)) {
        return -1; // 获取失败
    }
    
    // 注意：GetThreadGroupAffinity 告诉我们线程*可以*运行在哪些核心，而不是*当前*运行在哪。
    // Windows 没有直接等价于 Linux sched_getcpu() 的API。
    // 一种替代方案是使用性能计数器或依赖外部工具，但这很复杂。
    // 对于大多数目的，我们可能需要接受 Windows 下精确获取"瞬时"CPU ID 的困难。
    // 这里返回一个近似值或 -1 表示不支持精确查询。
    // 更实际的做法是记录亲和性掩码中允许的核心，但这不是"当前"核心。
    
    // 由于缺乏直接 API，这里演示如何获取允许的核心列表中的第一个作为近似（不推荐用于精确场景）
    // 实际应用中，可能需要依赖调试工具或特定库。
    // **重要**: Windows 原生 API 不直接暴露"当前运行核心"给用户态程序。
    return -1; // Windows 下无直接等效 API，返回 -1 表示不支持或未知
    
    // 替代思路：如果使用了 SetThreadIdealProcessorEx 设置了理想处理器，
    // 可以用 GetThreadIdealProcessorEx 查询，但这只是"理想"，不一定是"当前"。
    
#else
    // Linux/Unix 实现: 
    // 注意：Linux没有直接API根据线程句柄获取其他线程的当前CPU
    // sched_getcpu() 只能获取调用线程自身的CPU ID
    // 这里我们获取线程的亲和性掩码作为替代方案
    
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    
    // 获取指定线程的CPU亲和性
    int result = pthread_getaffinity_np(thread_handle, sizeof(cpu_set_t), &cpuset);
    if (result != 0) {
        return -1; // 获取失败
    }
    
    // 遍历CPU集合，找到第一个被设置的核心
    int num_cores = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
    for (int i = 0; i < num_cores; ++i) {
        if (CPU_ISSET(i, &cpuset)) {
            return i; // 返回亲和性掩码中的第一个核心
        }
    }
    return -1; // 未找到
#endif
}

/**
 * 获取当前调用线程正在运行的CPU核心ID。
 * 注意：此函数只能获取调用线程自身的CPU ID，无需传入线程句柄。
 * @return 成功时返回CPU核心ID (>=0)，失败时返回 -1。
 */
int get_current_thread_cpu_id() {
#ifdef _WIN32
    // Windows 实现：直接获取当前线程运行的处理器编号
    return static_cast<int>(GetCurrentProcessorNumber());
#else
    // Linux/Unix 实现: 使用 sched_getcpu()
    // sched_getcpu() 直接返回调用线程当前运行在哪个CPU上
    return sched_getcpu();
#endif
}

#endif // PLIB_CORE_UTILS_CPU_AFFINITY_HPP_