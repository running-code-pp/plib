/**
 * @Author: running-code-pp
 * @Date: 2025-10-23 13:56:20
 * @LastEditors: running-code-pp
 * @LastEditTime: 2025-10-23 13:57:04
 * @FilePath: \plib\benchmark\thread_pool_benchmark.cpp
 * @Description:对于线程池的基准测试，主要测试同样的任务消费的速度，以及线程空闲的时间
 * @Copyright: Copyright (c) 2025 by running-code-pp 3320996652@qq.com, All Rights Reserved.
 */
#include "utils/thread_pool.hpp"
#include "comparison/Comparison_threadpool.hpp"
#include <benchmark/benchmark.h>
using namespace plib::core::utils;

// plib中的线程池的基准测试
static void PLIB_threadpool_BENCHMARK(benchmark::State &state)
{
    ThreadPool<option_t::NONE> pool(std::thread::hardware_concurrency());

    for (auto _ : state)
    {
        std::atomic<int> counter = 0;
        int task_count = state.range(0);
        for (int i = 0; i < task_count; ++i)
        {
            pool.execute([&counter]()
                         { counter.fetch_add(1, std::memory_order_relaxed); });
        }
        // 等待所有任务完成
        while (counter.load(std::memory_order_relaxed) < task_count)
        {
            std::this_thread::yield();
        }
        benchmark::DoNotOptimize(counter.load());
    }
}

static void Other_threadpool_BENCHMARK(benchmark::State &state)
{
    comparison::ThreadPool pool(std::thread::hardware_concurrency());

    for (auto _ : state)
    {
        std::atomic<int> counter = 0;
        int task_count = state.range(0);
        for (int i = 0; i < task_count; ++i)
        {
            pool.enqueue([&counter]()
                         { counter.fetch_add(1, std::memory_order_relaxed); });
        }
        // 等待所有任务完成
        while (counter.load(std::memory_order_relaxed) < task_count)
        {
            std::this_thread::yield();
        }
        benchmark::DoNotOptimize(counter.load());
    }
}

// Register benchmarks，三个量级的任务数
BENCHMARK(PLIB_threadpool_BENCHMARK)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(Other_threadpool_BENCHMARK)->Arg(100)->Arg(1000)->Arg(10000);

BENCHMARK_MAIN();