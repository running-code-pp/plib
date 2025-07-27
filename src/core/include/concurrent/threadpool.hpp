#ifndef PLIB_CORE_CONCURRENT_THREADPOOL_HPP
#define PLIB_CORE_CONCURRENT_THREADPOOL_HPP
#include<stdint.h>
#include<thread>
#include<future>
#include<functional>
#include<type_traits>
#include<condition_variable>
#include<semaphore>
#include<compare>
#include<queue>
#include<mutex>
#include<memory>
#include<shared_mutex>
#include<chrono>
#include <concepts>
#include <exception>
#include <stdexcept>
#include <assert.h>

namespace plib::core::concurrent {
	enum  priority_t :std::int8_t {
		lowest = -128,
		low = -64,
		normal = 0,
		high = 64,
		highest = 127
	};

	enum  option_t :std::uint8_t {
		none = 0,//default
		priority = 1 << 0,//open task priority support
		pause = 1 << 2//open pause thread support
	};

	using task = std::function<void()>;

	struct task_t {
		task_t(std::function<void()> func, priority_t priority = priority_t::normal) :_func(func), _priority(priority) {}
		std::function<void()> _func;
		priority_t _priority;

		friend bool operator<(const task_t& lhs, const task_t& rhs) noexcept { return lhs._priority < rhs._priority; }
	};

	template<typename T>
	class multi_future :public std::vector<std::future<T>> {
	public:
		// to reuse the parent class constructor and deconstructor
		using std::vector<std::future<T>>::vector;
		auto get() {
			if constexpr (std::is_void_v<T>) {
				for (auto& f : *this) {
					f.get();
				}
				return;
			}
			else {
				std::vector<T> results;
				results.reserve(this->size());
				for (auto& f : *this) {
					results.emplace_back(f.get());
				}
				return results;
			}
		}

		template<typename R, typename P>
		[[nodiscard]] bool wait_for(std::chrono::duration<R, P> timeout) {
			auto remaining = timeout;
			for (auto& f : *this) {
				if (remaining < std::chrono::duration<R, P>::zero()) {
					return false;
				}
				auto start_this = std::chrono::steady_clock::now();
				if (f.wait_for(remaining) == std::future_status::timeout) {
					return false;
				}
				auto elapsed_this = std::chrono::steady_clock::now() - start_this;
				remaining -= elapsed_this;
			}
			return true;
		}

		[[nodiscard]] bool valid() const noexcept
		{
			bool is_valid = true;
			for (const std::future<T>& future : *this)
				is_valid = is_valid && future.valid();
			return is_valid;
		}

		void wait() const {
			for (const auto& f : *this) {
				f.wait();
			}
		}

		template <typename C, typename D>
		bool wait_until(const std::chrono::time_point<C, D>& t) const
		{
			for (const auto& f : *this) { f.wait_until(t); if (t < std::chrono::steady_clock::now()) return false; } return true;
		}

		[[nodiscard]] std::size_t ready_count() const
		{
			std::size_t count = 0;
			for (const std::future<T>& future : *this)
			{
				if (future.wait_for(std::chrono::duration<double>::zero()) == std::future_status::ready)
					++count;
			}
			return count;
		}
	};

	template<option_t opt = option_t::none>
	class ThreadPool {
	public:
		static constexpr bool priority_enabled = (opt & option_t::priority) != 0;
		static constexpr bool pause_enabled = (opt & option_t::pause) != 0;

		// disabled copy
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		ThreadPool& operator=(ThreadPool&&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;
		ThreadPool() : ThreadPool(0, [] {}) {}
		explicit ThreadPool(std::size_t n) : ThreadPool(n, [](size_t index) {}) {}

		template <typename F>
		explicit ThreadPool(F&& init) : ThreadPool(0, std::forward<F>(init)) {}

		template <typename F>
		ThreadPool(std::size_t n, F&& init)
		{
			create_threads(n, std::forward<F>(init));
		}

		void destroy_threads()
		{
			for (std::size_t i = 0; i < _threadNum; ++i)
				_threads[i].request_stop();

			// confirm before this operation all shared resource operation has done
			{
				const std::scoped_lock tasks_lock(_mutex);
			}
			_cv_taskAvailable.notify_all();
		}

		~ThreadPool() {
			try {
				wait();
				destroy_threads();
			}
			catch (...) {}
		}

		std::size_t get_tasks_queued() const {
			const std::scoped_lock tasks_lock(_mutex);
			return _taskQueue.size();
		}

		std::size_t get_tasks_running() const {
			const std::scoped_lock tasks_lock(_mutex);
			return _runningTaskNum;
		}

		std::size_t get_tasks_total() const {
			const std::scoped_lock tasks_lock(_mutex);
			return _runningTaskNum + _taskQueue.size();
		}

		std::size_t get_thread_count() const noexcept {
			return _threadNum;
		}

		std::vector<std::jthread::id> get_thread_ids() const {
			std::vector<std::jthread::id> thread_ids(_threadNum);
			for (std::size_t i = 0; i < _threadNum; ++i)
				thread_ids[i] = _threads[i].get_id();
			return thread_ids;
		}

		bool is_paused() const {
			const std::scoped_lock tasks_lock(_mutex);
			return _paused;
		}

		void pause() {
			const std::scoped_lock tasks_lock(_mutex);
			_paused = true;
		}

		void unpause() {
			{
				const std::scoped_lock tasks_lock(_mutex);
				_paused = false;
			}
			_cv_taskAvailable.notify_all();
		}

		void purge() {
			const std::scoped_lock tasks_lock(_mutex);
			_taskQueue = {};
		}

		void wait() {
			std::unique_lock tasks_lock(_mutex);
			_waiting = true;
			_cv_taskDone.wait(tasks_lock,
				[this]
				{
					if constexpr (pause_enabled)
						return (_runningTaskNum == 0) && (_paused || _taskQueue.empty());
					else
						return (_runningTaskNum == 0) && _taskQueue.empty();
				});
			_waiting = false;
		}

		template <typename R, typename P>
		bool wait_for(const std::chrono::duration<R, P>& duration) {
			std::unique_lock tasks_lock(_mutex);
			_waiting = true;
			const bool status = _cv_taskDone.wait_for(tasks_lock, duration,
				[this]
				{
					if constexpr (pause_enabled)
						return (_runningTaskNum == 0) && (_paused || _taskQueue.empty());
					else
						return (_runningTaskNum == 0) && _taskQueue.empty();
				});
			_waiting = false;
			return status;
		}

		template <typename C, typename D>
		bool wait_until(const std::chrono::time_point<C, D>& timeout_time) {
			std::unique_lock tasks_lock(_mutex);
			_waiting = true;
			const bool status = _cv_taskDone.wait_until(tasks_lock, timeout_time,
				[this]
				{
					if constexpr (pause_enabled)
						return (_runningTaskNum == 0) && (_paused || _taskQueue.empty());
					else
						return (_runningTaskNum == 0) && _taskQueue.empty();
				});
			_waiting = false;
			return status;
		}

		void reset() {
			reset(0, [](std::size_t) {});
		}

		void reset(const std::size_t num_threads) {
			reset(num_threads, [](std::size_t) {});
		}

		template <typename F>
		void reset(F&& init) {
			reset(0, std::forward<F>(init));
		}

		template <typename F>
		void reset(const std::size_t num_threads, F&& init) {
			if constexpr (pause_enabled) {
				std::unique_lock tasks_lock(_mutex);
				const bool was_paused = _paused;
				_paused = true;
				tasks_lock.unlock();
				reset_pool(num_threads, std::forward<F>(init));
				tasks_lock.lock();
				_paused = was_paused;
			}
			else {
				reset_pool(num_threads, std::forward<F>(init));
			}
		}

		//template <typename F>
		//void set_cleanup_func(F&& cleanup) {
		//	if constexpr (std::is_invocable_v<F, std::size_t>) {
		//		_cleanFunc = std::forward<F>(cleanup);
		//	}
		//	else {
		//		_cleanFunc = [cleanup = std::forward<F>(cleanup)](std::size_t) {
		//			cleanup();
		//			};
		//	}
		//}

		template<typename F>
		void create_threads(size_t n, F&& init) {
			if constexpr (std::is_invocable_v<F, size_t>) {
				_initFunc = std::forward<F>(init);
			}
			else if constexpr (std::is_invocable_v<F>) {
				_initFunc = [init = std::forward<F>(init)](size_t) { init(); };
			}
			else {
				_initFunc = [](size_t) {};
			}
			_threadNum = n > 0 ? n : (std::jthread::hardware_concurrency() > 0 ? std::jthread::hardware_concurrency() : 1);

			_threads = std::make_unique<std::jthread[]>(_threadNum);

			{
				std::scoped_lock tasks_lock(_mutex);
				_runningTaskNum = _threadNum;
			}
			for (std::size_t i = 0; i < _threadNum; ++i) {
				_threads[i] = std::jthread([this, i](const std::stop_token& stop_token) { worker(stop_token, i); });
			}
		}

		template<typename F>
		void reset_pool(const std::size_t num_threads, F&& init) {
			wait();
			destroy_threads();
			create_threads(num_threads, std::forward<F>(init));
		}

		template<typename F>
		void detach_task(F&& task, const priority_t priority = priority_t::normal) {
			{
				const std::scoped_lock sl(_mutex);
				if constexpr (priority_enabled) {
					_taskQueue.emplace(std::forward<F>(task), priority);
				}
				else {
					_taskQueue.emplace(std::forward<F>(task));
				}
			}
			_cv_taskAvailable.notify_one();
		}

		template <typename F, typename R = std::invoke_result_t<std::decay_t<F>>>
		[[nodiscard]] std::future<R> submit_task(F&& task_, priority_t priority = priority_t::normal)
		{
			auto promise_ptr = std::make_shared<std::promise<R>>();
			std::future<R> future = promise_ptr->get_future();
			detach_task(
				[task = std::forward<F>(task_), promise_ptr]() mutable {
					try {
						if constexpr (std::is_void_v<R>) {
							task();
							promise_ptr->set_value();
						}
						else {
							promise_ptr->set_value(task());
						}
					}
					catch (...) {
						try {
							promise_ptr->set_exception(std::current_exception());
						}
						catch (...) {}
					}
				},
				priority);
			return future;
		}

	protected:
		task_t pop_task() {
			task task_;
			if constexpr (priority_enabled) {
				task_ = std::move(const_cast<task_t&>(_taskQueue.top())._func);
			}
			else {
				task_ = std::move(_taskQueue.front());
			}
			_taskQueue.pop();
			return task_;
		}

		void worker(std::stop_token stop_token, size_t index) {
			if (_initFunc) {
				_initFunc(index);
			}
			while (true) {
				std::unique_lock tasks_lock(_mutex);
				--_runningTaskNum;
				if constexpr (pause_enabled) {
					if (_waiting && (_runningTaskNum == 0) && (_paused || _taskQueue.empty())) {
						_cv_taskDone.notify_all();
					}
				}
				else {
					if (_waiting && (_runningTaskNum == 0) && _taskQueue.empty()) {
						_cv_taskDone.notify_all();
					}
				}
				_cv_taskAvailable.wait(tasks_lock, stop_token, [this, &stop_token]() -> bool {
					if (stop_token.stop_requested()) return true;

					if constexpr (pause_enabled) {
						return !(_paused || _taskQueue.empty());
					}
					else {
						return !_taskQueue.empty();
					}});
					// request pause
					if (stop_token.stop_requested()) {
						break;
					}

					{
						task_t task_ = pop_task();
						++_runningTaskNum;
						tasks_lock.unlock();
						try
						{
							task_._func();
						}
						catch (...)
						{
						}
					}
			}
			if (_cleanupFunc) {
				_cleanupFunc(index);
			}
		}

	private:
		std::function<void(size_t index)>_initFunc, _cleanupFunc;
		std::conditional_t < priority_enabled, std::priority_queue<task_t>, std::queue<task>> _taskQueue;
		std::unique_ptr<std::jthread[]> _threads;
		mutable std::mutex _mutex;
		bool _paused = {};
		std::condition_variable_any _cv_taskAvailable;
		std::condition_variable _cv_taskDone;
		bool _waiting = false;
		size_t _threadNum;
		size_t _runningTaskNum;
		size_t _remainnig;
	};
} //namespace plib::core::concurrent
#endif