/**
 * @Author: running-code-pp
 * @Date: 2025-10-22 14:13:09
 * @LastEditors: running-code-pp
 * @LastEditTime: 2025-10-23 14:38:57
 * @FilePath: \plib\src\core\include\utils\thread_pool.hpp
 * @Description:
 * @Copyright: Copyright (c) 2025 by running-code-pp 3320996652@qq.com, All Rights Reserved.
 */

/**
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-22 14:13:09
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-22 14:13:58
 * @FilePath: \plib\src\core\include\utils\thread_pool.hpp
 * @Description: 线程池实现，支持绑定cpu核心，支持优先级,通过多个任务队列来降低锁的粒度，不必所有线程都被一个全局任务队列限制，通过try操作来减少阻塞
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
#ifndef PLIB_CORE_UTILS_THREAD_POOL_HPP_
#define PLIB_CORE_UTILS_THREAD_POOL_HPP_

#include <cstdint>
#include <cstddef>
#include <thread>
#include <condition_variable>
#include <queue>
#include <functional>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>
#include <atomic>
#include <chrono>
#include "utils/cpu_affinity.hpp"
#include "type/threadsafe_queue.hpp"
#include "plib_macros.hpp"

namespace plib::core::utils
{
    /**
     *@brief: 任务优先级枚举，数值越大优先级越高
     */
    enum priority_t : std::int8_t
    {
        lowest = -128,
        low = -64,
        normal = 0,
        high = 64,
        highest = 127
    };

    // 暂时的任务,
    // TODO: 使用 move_only_function 替代 std::function 减去RTTI和拷贝的开销
    using TASK = std::function<void()>;

    struct TaskItem
    {
        TASK task;
        priority_t priority;

        // 重载三路比较运算符
        // 注意：priority_queue 是大顶堆，所以要反向比较让高优先级排在前面
        auto operator<=>(const TaskItem &other) const noexcept
        {
            return other.priority <=> priority; // 反向比较，让高优先级的任务排在前面
        }
    };

    enum option_t : std::uint8_t
    {
        NONE,
        PRIORITY
    };

    namespace detail
    {
        // 普通模式的工作函数
        template <option_t opt>
        struct WorkerImpl
        {
            template <typename PoolType>
            static void work(PoolType *pool, int core_index)
            {
                // 触发开始钩子
                if (pool->_start_hook)
                {
                    pool->_start_hook();
                }
                for (;;)
                {
                    TASK task;
                    bool found = false;

                    // 尝试从所有队列获取任务
                    for (auto &task_queue : pool->_task_queues)
                    {
                        if (task_queue.try_pop(task))
                        {
                            found = true;
                            break;
                        }
                    }

                    // 检查是否需要停止
                    if (pool->_stop.load(std::memory_order_acquire))
                    {
                        break;
                    }

                    // 如果没有获取到任务，从自己的队列阻塞等待
                    if (!found)
                    {
                        if (pool->_task_queues.empty())
                        {
                            printf("task queues empty, thread exiting\n");
                            // 如果没有任务队列，直接跳出循环
                            break;
                        }
                        // 确保索引在范围内
                        int queue_index = core_index % static_cast<int>(pool->_task_queues.size());
                        pool->_task_queues[queue_index].pop(task);

                        // pop 返回后再次检查停止状态
                        if (pool->_stop.load(std::memory_order_acquire))
                        {
                            break;
                        }
                        found = true;
                    }

                    if (found && task)
                    {
                        task();
                        task = nullptr; // 重置任务
                    }
                }
                // 触发退出钩子
                if (pool->_exit_hook)
                {
                    pool->_exit_hook();
                }
            }
        };

        // 优先级模式的特化
        template <>
        struct WorkerImpl<option_t::PRIORITY>
        {
            template <typename PoolType>
            static void work(PoolType *pool, int core_index)
            {
                // 触发开始钩子
                if (pool->_start_hook)
                {
                    pool->_start_hook();
                }
                for (;;)
                {
                    TaskItem task_item;
                    bool found = false;

                    // 尝试从所有队列获取任务
                    for (auto &task_queue : pool->_task_queues)
                    {
                        if (task_queue.try_pop(task_item))
                        {
                            found = true;
                            break;
                        }
                    }

                    // 检查是否需要停止
                    if (pool->_stop.load(std::memory_order_acquire))
                    {
                        break;
                    }

                    // 如果没有获取到任务，从自己的队列阻塞等待
                    if (!found)
                    {
                        // 确保索引在范围内
                        int queue_index = core_index % static_cast<int>(pool->_task_queues.size());
                        pool->_task_queues[queue_index].pop(task_item);

                        // pop 返回后再次检查停止状态
                        if (pool->_stop.load(std::memory_order_acquire))
                        {
                            break;
                        }
                        found = true;
                    }

                    if (found && task_item.task)
                    {
                        task_item.task();
                        task_item.task = nullptr; // 重置任务
                    }
                }
                // 触发退出钩子
                if (pool->_exit_hook)
                {
                    pool->_exit_hook();
                }
            }
        };
    } // namespace detail

    /**
     * @brief: 线程池类，支持任务优先级和CPU核心绑定,默认支持cpu特性
     */
    template <option_t opt = option_t::NONE>
    class ThreadPool
    {
        // 编译器计算是否开启优先级支持
        static constexpr bool priority_enabled = (opt & option_t::PRIORITY) != 0;

    public:
        /**
         * @param thread_num: 线程池中的线程数
         * @param cpu_binding: 是否绑定cpu核心,提升cpu亲和性
         */
        ThreadPool(std::size_t thread_num, bool cpu_binding = false);
        ~ThreadPool();

        // 当启用优先级时的execute函数
        template <option_t opt1 = opt, typename std::enable_if_t<(opt1 & option_t::PRIORITY) != 0, int> = 0>
        void execute(TASK &&task, priority_t priority = priority_t::normal);

        // 当不启用优先级时的execute函数
        template <option_t opt1 = opt, typename std::enable_if_t<(opt1 & option_t::PRIORITY) == 0, int> = 0>
        void execute(TASK &&task, int idx = -1);

        /**
         * @brief: 停止线程池，停止之后不会再接受新任务，剩余未开始执行的任务也不会再执行,所有线程都会退出
         */
        void stop();

    public:
        void set_start_hook(const std::function<void()> &hook);
        void set_exit_hook(const std::function<void()> &hook);

    private:
        std::size_t _thread_num;
        // 线程安全的队列
        std::vector<std::jthread> _threads;
        bool _cpu_binding;
        std::atomic<bool> _stop;

        template <option_t>
        friend struct detail::WorkerImpl;
        std::function<void()> _start_hook; // 线程开始的钩子
        std::function<void()> _exit_hook;  // 线程退出的钩子

    public:
        std::vector<std::conditional_t<priority_enabled, plib::core::type::ThreadSafePriorityQueue<TaskItem>,
                                       plib::core::type::ThreadSafeQueue<TASK>>>
            _task_queues;
    };

    template <option_t opt>
    void ThreadPool<opt>::set_start_hook(const std::function<void()> &hook)
    {
        _start_hook = hook;
    }

    template <option_t opt>
    void ThreadPool<opt>::set_exit_hook(const std::function<void()> &hook)
    {
        _exit_hook = hook;
    }

    template <option_t opt>
    ThreadPool<opt>::ThreadPool(std::size_t thread_num, bool cpu_binding)
        : _thread_num([&]() -> std::size_t
                      {
				if (thread_num == 0) {
					auto hw_concurrency = std::thread::hardware_concurrency();
					return (hw_concurrency == 0) ? 1 : hw_concurrency;
				}
				return thread_num; }()),
          _threads(),
          _cpu_binding(cpu_binding),
          _stop(false),
          _task_queues(_thread_num),
          _start_hook(nullptr),
          _exit_hook(nullptr)
    {
        // 初始化线程池
        _threads.reserve(_thread_num);
        for (int i = 0; i < _thread_num; ++i)
        {
            _threads.emplace_back([this, i]()
                                  { detail::WorkerImpl<opt>::work(this, i); });
        }

        if (_cpu_binding)
        {
            for (int i = 0; i < _thread_num; ++i)
                set_thread_affinity(_threads[i].native_handle(), i % std::thread::hardware_concurrency());
        }
    }

    template <option_t opt>
    ThreadPool<opt>::~ThreadPool()
    {
        // jthread会自动join
        stop();
    }

    // 启用优先级时的实现
    template <option_t opt>
    template <option_t opt1, typename std::enable_if_t<(opt1 & option_t::PRIORITY) != 0, int>>
    void ThreadPool<opt>::execute(TASK &&task, priority_t priority)
    {
        // // 将任务包装成 TaskItem 推入优先级队列
        // TaskItem task_item{std::move(task), priority};
        // _task_queues[0].push(std::move(task_item));
        for (auto &queue : _task_queues)
        {
            if (queue.try_push(TaskItem{std::move(task), priority}))
                return;
        }
        // 如果没有成功入队，则随机挑选一个队列入队
        _task_queues[std::rand() % _thread_num].push(TaskItem{std::move(task), priority});
    }

    // 不启用优先级时的实现
    template <option_t opt>
    template <option_t opt1, typename std::enable_if_t<(opt1 & option_t::PRIORITY) == 0, int>>
    void ThreadPool<opt>::execute(TASK &&task, int idx)
    {
        P_LIKELY if (idx == -1)
        {
            // 提交到默认队列
            _task_queues[0].push(std::move(task));
            return;
        }
        // 提交到指定队列
        _task_queues[idx].push(std::move(task));
    }

    template <option_t opt>
    void ThreadPool<opt>::stop()
    {
        _stop = true;
        for (auto &queue : _task_queues)
            queue.stop();
    }
} // namespace plib::core::utils
#endif // PLIB_CORE_UTILS_THREAD_POOL_HPP_