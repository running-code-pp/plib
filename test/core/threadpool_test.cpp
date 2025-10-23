/**
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-10 22:12:40
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-22 18:07:55
 * @FilePath: \plib\unittest\core\threadpool_test.cpp
 * @Description: 线程池测试
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
#include <cstdio>
#include <chrono>
#include <thread>
#include <random>
#include "utils/thread_pool.hpp"
using namespace plib::core::utils;

plib::core::utils::priority_t generateRandomPriority()
{
    using priority = plib::core::utils::priority_t;
    static std::vector<priority> _dict = {priority::lowest, priority::low, priority::normal, priority::high, priority::highest};
    return _dict[rand() % _dict.size()];
}

int main()
{
    // std::thread::hardware_concurrency()
    ThreadPool<option_t::NONE> pool(std::thread::hardware_concurrency(), true);
    ThreadPool<option_t::PRIORITY> priority_pool(std::thread::hardware_concurrency(), true);
    int start_count = 0, exit_count = 0;

    auto start_hook = [&start_count]()
    {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        printf("Thread started, total started threads: %d\n", ++start_count);
    };

    auto exit_hook = [&exit_count]()
    {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        printf("Thread exiting, total exited threads: %d\n", ++exit_count);
    };

    pool.set_start_hook(start_hook);
    pool.set_exit_hook(exit_hook);

    for (int i = 0; i < 100; i++)
    {
        pool.execute([i]()
                     {
            printf("Task %d is running\n", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            printf("Task %d is finished\n", i); });
    }

    for (int i = 0; i < 10; i++)
    {
        priority_pool.execute([i]()
                              {
            printf("Priority Task %d is running\n", i);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            printf("Priority Task %d is finished\n", i); }, generateRandomPriority());
    }

    // priority_pool.stop();
    char c = getchar(); // 阻塞主线程
    printf("Main thread received input: %c\n", c);
    pool.stop();
    priority_pool.stop();
    return 0;
}
