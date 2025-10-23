/**
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-22 14:50:05
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-22 14:52:13
 * @FilePath: \plib\src\core\include\type\threadsafe_queue.hpp
 * @Description: 线程安全的同步队列/优先级队列，支持try类型的操作
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */

#ifndef PLIB_CORE_UTILS_THREADSAFE_QUEUE_HPP_
#define PLIB_CORE_UTILS_THREADSAFE_QUEUE_HPP_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <concepts>
#include "plib_macros.hpp"

namespace plib::core::type
{
    /**
     * @brief: 支持try操作的queue的线程安全版本
     */
    template <typename T>
        requires std::is_move_assignable_v<T>
    class ThreadSafeQueue
    {
    public:
        bool try_push(T &&value)
        {
            {
                std::unique_lock lock(_mtx, std::try_to_lock);
                if (!lock)
                    return false; // 当前其他线程正在占用锁直接return
                _queue.push(std::move(value));
            }
            _cv.notify_one();
            return true;
        }

        void push(T &&value)
        {
            {
                std::lock_guard lock(_mtx);
                _queue.push(std::move(value));
            }
            _cv.notify_one();
        }

        bool try_pop(T &value)
        {
            std::unique_lock lock(_mtx, std::try_to_lock);
            if (!lock || _queue.empty())
                return false; // 当前其他线程正在占用锁直接return
            value = std::move(_queue.front());
            _queue.pop();
            return true;
        }

        void pop(T &value)
        {
            std::unique_lock lock(_mtx);
            _cv.wait(lock, [this]()
                     { return !_queue.empty() || _stop; });
            P_LIKELY if (!_stop)
            {
                value = std::move(_queue.front());
                _queue.pop();
            }
        }

        std::size_t size() const
        {
            std::lock_guard lock(_mtx);
            return _queue.size();
        }
        // 停止队列，通过条件变量唤醒所有等待的线程
        void stop()
        {
            {
                std::scoped_lock guard(_mtx);
                _stop = true;
            }
            _cv.notify_all();
        }

    private:
        bool _stop = false;
        std::queue<T> _queue;
        mutable std::mutex _mtx;
        std::condition_variable _cv;
    };

    /**
     * @brief: 支持try操作的priority_queue的线程安全版本
     */
    template <typename T>
        requires std::is_move_assignable_v<T>
    class ThreadSafePriorityQueue
    {
    public:
        bool try_push(T &&value)
        {
            {
                std::unique_lock lock(_mtx, std::try_to_lock);
                if (!lock)
                    return false; // 当前其他线程正在占用锁直接return
                _queue.push(std::move(value));
            }
            _cv.notify_one();
            return true;
        }

        void push(T &&value)
        {
            {
                std::lock_guard lock(_mtx);
                _queue.push(std::move(value));
            }
            _cv.notify_one();
        }

        bool try_pop(T &value)
        {
            std::unique_lock lock(_mtx, std::try_to_lock);
            if (!lock || _queue.empty())
                return false; // 当前其他线程正在占用锁直接return
            value = std::move(_queue.top());
            _queue.pop();
            return true;
        }

        void pop(T &value)
        {
            std::unique_lock lock(_mtx);
            _cv.wait(lock, [this]()
                     { return !_queue.empty() || _stop; });
            P_LIKELY if (!_stop && !_queue.empty())
            {
                value = std::move(const_cast<T &>(_queue.top()));
                _queue.pop();
            }
        }

        std::size_t size() const
        {
            std::lock_guard lock(_mtx);
            return _queue.size();
        }

        // 停止队列，通过条件变量唤醒所有等待的线程
        void stop()
        {
            {
                std::scoped_lock guard(_mtx);
                _stop = true;
            }
            _cv.notify_all();
        }

    private:
        bool _stop = false;
        std::priority_queue<T> _queue;
        std::mutex _mtx;
        std::condition_variable _cv;
    };
} // namespace plib::core::type

#endif // PLIB_CORE_UTILS_THREADSAFE_QUEUE_HPP_