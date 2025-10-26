/**
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-26 20:39:49
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-26 20:41:56
 * @FilePath: \plib\src\core\include\concurrent\spinlock.hpp
 * @Description: 几种自旋锁的实现
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */

#ifndef PLIB_CORE_CONCURRENT_SPINLOCK_HPP_
#define PLIB_CORE_CONCURRENT_SPINLOCK_HPP_

#include <atomic>
#include <thread>
#include <mutex>
#include "plib_macros.hpp"

namespace plib::core::concurrent
{

    /**
     * @brief: 基于原子变量的简单自旋锁实现,适用于等待时间短，竞争小的场景
     */
    class Spinlock
    {
    public:
        Spinlock() : flag(ATOMIC_FLAG_INIT) {}

        void lock()
        {
            while (flag.test_and_set(std::memory_order_acquire))
            {
            }
        }

        void unlock()
        {
            flag.clear(std::memory_order_release);
        }

    private:
        std::atomic_flag flag;
    };

    /**
     * @brief: FIFO公平自旋锁，适用于高竞争场景
     */
    class FairSpinLock
    {
    public:
        FairSpinLock() : _ticket(0), _next(0) {}

        P_NOTINLINE unsigned int lock()
        {
            auto ticket = _ticket.fetch_add(1, std::memory_order_relaxed);

            for (;;)
            {
                auto position = ticket - _next.load(std::memory_order_acquire);
                P_LIKELY if (!position)
                {
                    break;
                }
                do
                {
                    CPU_PAUSE();
                } while (--position);
            }
            return ticket;
        }

        void unlock()
        {
            unlock(_next.load(std::memory_order_relaxed) + 1);
        }

        void unlock(unsigned int ticket)
        {
            _next.store(ticket + 1, std::memory_order_release);
        }

    private:
        // 缓存行对齐，避免伪共享
        alignas(CACHE_LINE_SIZE) std::atomic<unsigned int> _ticket;
        alignas(CACHE_LINE_SIZE) std::atomic<unsigned int> _next;
    };

    /**
     * @brief: 非公平的自旋锁,是第一种自旋锁实现的优化版本，适用于大多数场景的一种中庸实现
     */
    class UnfairSpinlock
    {
    public:
        UnfairSpinlock(UnfairSpinlock &) = delete;
        UnfairSpinlock &operator=(UnfairSpinlock &) = delete;

        void lock()
        {
            for (;;)
            {
                if (!_lock.load(std::memory_order_relaxed) &&
                    !_lock.exchange(1, std::memory_order_acquire))
                {
                    return;
                }
                CPU_PAUSE();
            }
        }

        void unlock()
        {
            _lock.store(0, std::memory_order_release);
        }

    private:
        std::atomic<unsigned int> _lock{0};
    };

} // namespace plib::core::concurrent

#endif // PLIB_CORE_CONCURRENT_SPINLOCK_HPP_