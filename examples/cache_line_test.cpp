/**
 * @brief 缓存行大小测试和演示
 * 展示不同架构下缓存行大小对性能的影响
 */

#include "concurrent/spinlock.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

using namespace plib::core::concurrent;

// 演示伪共享问题
struct WithoutAlignment {
    std::atomic<int> counter1{0};
    std::atomic<int> counter2{0};  // 可能与counter1在同一缓存行
};

struct WithAlignment {
    alignas(CACHE_LINE_SIZE) std::atomic<int> counter1{0};
    alignas(CACHE_LINE_SIZE) std::atomic<int> counter2{0};  // 强制在不同缓存行
};

void print_architecture_info() {
    std::cout << "=== 架构信息 ===" << std::endl;
    
    #if defined(__x86_64__) || defined(_M_X64)
        std::cout << "架构: x86-64 (64位)" << std::endl;
    #elif defined(__i386__) || defined(_M_IX86)
        std::cout << "架构: x86 (32位)" << std::endl;
    #elif defined(__aarch64__) || defined(_M_ARM64)
        std::cout << "架构: ARM64 (AArch64)" << std::endl;
    #elif defined(__arm__) || defined(_M_ARM)
        std::cout << "架构: ARM (32位)" << std::endl;
    #elif defined(__powerpc__) || defined(__ppc__)
        std::cout << "架构: PowerPC" << std::endl;
    #else
        std::cout << "架构: 未知" << std::endl;
    #endif
    
    std::cout << "缓存行大小: " << CACHE_LINE_SIZE << " 字节" << std::endl;
    
    #ifdef __cpp_lib_hardware_interference_size
        std::cout << "使用 C++17 hardware_interference_size" << std::endl;
        std::cout << "  - destructive_interference_size: " 
                  << std::hardware_destructive_interference_size << " 字节" << std::endl;
        std::cout << "  - constructive_interference_size: " 
                  << std::hardware_constructive_interference_size << " 字节" << std::endl;
    #else
        std::cout << "使用编译时架构检测" << std::endl;
    #endif
    
    std::cout << std::endl;
}

template<typename T>
void false_sharing_test(T& data, const std::string& name) {
    constexpr int NUM_ITERATIONS = 1000000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // 两个线程同时修改不同的计数器
    std::thread t1([&]() {
        for (int i = 0; i < NUM_ITERATIONS; ++i) {
            data.counter1.fetch_add(1, std::memory_order_relaxed);
        }
    });
    
    std::thread t2([&]() {
        for (int i = 0; i < NUM_ITERATIONS; ++i) {
            data.counter2.fetch_add(1, std::memory_order_relaxed);
        }
    });
    
    t1.join();
    t2.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << name << ": " << duration.count() << " μs" 
              << " (counter1=" << data.counter1 << ", counter2=" << data.counter2 << ")"
              << std::endl;
}

void spinlock_performance_test() {
    std::cout << "=== 自旋锁性能测试 ===" << std::endl;
    
    constexpr int NUM_THREADS = 4;
    constexpr int NUM_ITERATIONS = 100000;
    
    // 测试简单自旋锁
    {
        Spinlock lock;
        volatile int counter = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < NUM_ITERATIONS; ++j) {
                    lock.lock();
                    ++counter;
                    lock.unlock();
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "简单自旋锁: " << duration.count() << " μs (counter=" << counter << ")" << std::endl;
    }
    
    // 测试票据自旋锁
    {
        TicketSpinLock lock;
        volatile int counter = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < NUM_ITERATIONS; ++j) {
                    lock.lock();
                    ++counter;
                    lock.unlock();
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "票据自旋锁: " << duration.count() << " μs (counter=" << counter << ")" << std::endl;
    }
    
    // 测试自适应自旋锁
    {
        AdaptiveSpinLock lock;
        volatile int counter = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < NUM_ITERATIONS; ++j) {
                    lock.lock();
                    ++counter;
                    lock.unlock();
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "自适应自旋锁: " << duration.count() << " μs (counter=" << counter << ")" << std::endl;
    }
    
    std::cout << std::endl;
}

int main() {
    print_architecture_info();
    
    std::cout << "=== 伪共享测试 ===" << std::endl;
    
    WithoutAlignment data1;
    WithAlignment data2;
    
    false_sharing_test(data1, "未对齐 (可能伪共享)");
    false_sharing_test(data2, "缓存行对齐      ");
    
    std::cout << std::endl;
    
    spinlock_performance_test();
    
    return 0;
}