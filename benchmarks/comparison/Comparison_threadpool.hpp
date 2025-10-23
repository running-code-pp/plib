/**
 * @Author: running-code-pp
 * @Date: 2025-10-23 13:30:49
 * @LastEditors: running-code-pp
 * @LastEditTime: 2025-10-23 13:55:27
 * @FilePath: \plib\benchmark\comparison\Comparison_threadpool.hpp
 * @Description:  开源线程池实现 github 2k+ star
 * @Copyright: Copyright (c) 2025 by running-code-pp 3320996652@qq.com, All Rights Reserved.
 */

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

namespace comparison
{
    class ThreadPool
    {
    public:
        ThreadPool(size_t);
        template <class F, class... Args>
        auto enqueue(F &&f, Args &&...args)
            -> std::future<typename std::invoke_result<F, Args...>::type>;
        ~ThreadPool();
        void set_start_hook(std::function<void()> hook) { _start_hook = hook; }
        void set_exit_hook(std::function<void()> hook) { _exit_hook = hook; }

    private:
        std::function<void()> _start_hook;
        std::function<void()> _exit_hook;
        // need to keep track of threads so we can join them
        std::vector<std::thread> workers;
        // the task queue
        std::queue<std::function<void()>> tasks;

        // synchronization
        std::mutex queue_mutex;
        std::condition_variable condition;
        bool stop;
    };

    // the constructor just launches some amount of workers
    inline ThreadPool::ThreadPool(size_t threads)
        : stop(false)
    {
        for (size_t i = 0; i < threads; ++i)
            workers.emplace_back(
                [this]
                {
                    if (this->_start_hook)
                    {
                        this->_start_hook();
                    }
                    for (;;)
                    {
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            this->condition.wait(lock,
                                                 [this]
                                                 { return this->stop || !this->tasks.empty(); });
                            if (this->stop && this->tasks.empty())
                                return;
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }

                        task();
                    }
                    if (this->_exit_hook)
                    {
                        this->_exit_hook();
                    }
                });
    }

    // add new work item to the pool
    template <class F, class... Args>
    auto ThreadPool::enqueue(F &&f, Args &&...args)
        -> std::future<typename std::invoke_result<F, Args...>::type>
    {
        using return_type = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            // don't allow enqueueing after stopping the pool
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            tasks.emplace([task]()
                          { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    // the destructor joins all threads
    inline ThreadPool::~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
            worker.join();
    }

}

#endif