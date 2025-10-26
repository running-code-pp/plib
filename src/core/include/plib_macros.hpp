#ifndef PLIB_CORE_PLIB_MACROS_HPP_
#define PLIB_CORE_PLIB_MACROS_HPP_

// cpu预测分支优化
#if __has_cpp_attribute(likely) && __has_cpp_attribute(unlikely)
#define P_LIKELY [[likely]]
#define P_UNLIKELY [[unlikely]]
#else
#define P_LIKELY
#define P_UNLIKELY
#endif

// 强制内联
#ifdef _WIN32
#define P_INLINE
#else
#define P_INLINE __attribute__((__always_inline__)) inline
#endif

// 避免编译器优化导致内联
#ifdef _WIN32
#define P_NOTINLINE
#else
#define P_NOTINLINE __attribute__((noinline))
#endif

// 编译期内存泄露检测
#ifdef __clang__
#if __has_feature(address_sanitizer)
#define P_INTERNAL_USE_ASAN 1
#endif  // __has_feature(address_sanitizer)
#endif  // __clang__

#ifdef __GNUC__
#ifdef __SANITIZE_ADDRESS__  // GCC
#define P_INTERNAL_USE_ASAN 1
#endif  // __SANITIZE_ADDRESS__
#endif  // __GNUC__

// 协程只有在执行完成的时候才销毁，clang和alibaba_clang支持
#if __has_cpp_attribute(clang::coro_only_destroy_when_complete)
#define CORO_ONLY_DESTROY_WHEN_DONE [[clang::coro_only_destroy_when_complete]]
#else
#define CORO_ONLY_DESTROY_WHEN_DONE
#endif

// cpu高速缓存 L1缓存行大小
#ifdef __cpp_lib_hardware_interference_size
    constexpr size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;
#else
    // 不同架构的缓存行大小
    #if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
        // x86/x64: 64 字节
        constexpr size_t CACHE_LINE_SIZE = 64;
    #elif defined(__aarch64__) || defined(_M_ARM64)
        // ARM64: 通常 64 或 128 字节，使用较大的值确保安全
        constexpr size_t CACHE_LINE_SIZE = 128;
    #elif defined(__arm__) || defined(_M_ARM)
        // ARM32: 通常 32 或 64 字节
        constexpr size_t CACHE_LINE_SIZE = 64;
    #elif defined(__powerpc__) || defined(__ppc__)
        // PowerPC: 通常 128 字节
        constexpr size_t CACHE_LINE_SIZE = 128;
    #else
        // 默认值：大多数现代处理器
        constexpr size_t CACHE_LINE_SIZE = 64;
    #endif
#endif


// 指示CPU当前线程在等待锁时可以释放一些CPU资源，从而让出CPU给其他线程使用。
// 这是一种硬件级别的让步机制，能够有效地减少CPU资源的浪费。
// 通常用于自旋锁忙等中
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #include <immintrin.h>  // for _mm_pause()
    #define CPU_PAUSE() _mm_pause()
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define CPU_PAUSE() __asm__ volatile("yield" ::: "memory")
#elif defined(__arm__) || defined(_M_ARM)
    #define CPU_PAUSE() __asm__ volatile("yield" ::: "memory")
#elif defined(__powerpc__) || defined(__ppc__)
    #define CPU_PAUSE() __asm__ volatile("or 27,27,27" ::: "memory")
#elif defined(__riscv)
    #define CPU_PAUSE() __asm__ volatile("pause" ::: "memory")
#else
    // 回退到标准yield，但这会让出CPU时间片
    #define CPU_PAUSE() std::this_thread::yield()
#endif


#endif // PLIB_CORE_PLIB_MACROS_HPP_
