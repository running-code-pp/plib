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
#define AS_INLINE
#else
#define P_INLINE __attribute__((__always_inline__)) inline
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

#endif // PLIB_CORE_PLIB_MACROS_HPP_
