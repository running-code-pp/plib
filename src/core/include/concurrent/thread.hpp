#ifndef PLIB_CORE_CONCURRENT_THREAD_HPP
#define PLIB_CORE_CONCURRENT_THREAD_HPP
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#include <semaphore>
#include <stop_token>

namespace plib::core::concurrent {
	using opt_t = std::uint8_t;
	template <opt_t> class thread_pool;
	using task_t = std::function<void()>;
	using thread_t = std::jthread;
	using priority_t = std::int8_t;

	enum pr : priority_t {
		lowest = -128,
		low = -64,
		normal = 0,
		high = +64,
		highest = +127
	};

	struct pr_task {
		explicit pr_task(task_t&& task_, const priority_t priority_ = 0) noexcept : task(std::move(task_)), priority(priority_) {}
		friend bool operator<(const pr_task& lhs, const pr_task& rhs) noexcept { return lhs.priority < rhs.priority; }
		task_t task;
		priority_t priority = 0;
	};

	template <typename T> class multi_future : public std::vector<std::future<T>> {
	public:
		using std::vector<std::future<T>>::vector;
		auto get() {
			if constexpr (std::is_void_v<T>) { for (auto& f : *this) f.get(); return; }
			else { std::vector<T> results; results.reserve(this->size()); for (auto& f : *this) results.push_back(f.get()); return results; }
		}
		std::size_t ready_count() const {
			std::size_t c = 0;
			for (const auto& f : *this)
				if (f.wait_for(std::chrono::duration<double>::zero()) == std::future_status::ready)
					++c;
			return c;
		}
		bool valid() const noexcept {
			bool v = true;
			for (const auto& f : *this)
				v = v && f.valid();
			return v;
		}
		void wait() const { for (const auto& f : *this) f.wait(); }
		template <typename R, typename P> bool wait_for(const std::chrono::duration<R, P>& d) const {
			auto start = std::chrono::steady_clock::now();
			auto remaining = d;

			for (const auto& f : *this) {
				if (remaining <= std::chrono::duration<R, P>::zero())
					return false;

				auto start_this = std::chrono::steady_clock::now();

				if (f.wait_for(remaining) == std::future_status::timeout)
					return false;

				auto elapsed_this = std::chrono::steady_clock::now() - start_this;
				remaining -= elapsed_this;
			}

			return true;
		}
		template <typename C, typename D>
		bool wait_until(const std::chrono::time_point<C, D>& t) const
		{
			for (const auto& f : *this) { f.wait_until(t); if (t < std::chrono::steady_clock::now()) return false; } return true;
		}
	};

	template <typename T1, typename T2, typename Enable = void> struct common_index_type { using type = std::common_type_t<T1, T2>; };
	template <typename T1, typename T2> struct common_index_type<T1, T2, std::enable_if_t<std::is_signed_v<T1>&& std::is_signed_v<T2>>> { using type = std::conditional_t<(sizeof(T1) >= sizeof(T2)), T1, T2>; };
	template <typename T1, typename T2> struct common_index_type<T1, T2, std::enable_if_t<std::is_unsigned_v<T1>&& std::is_unsigned_v<T2>>> { using type = std::conditional_t<(sizeof(T1) >= sizeof(T2)), T1, T2>; };
	template <typename T1, typename T2> struct common_index_type<T1, T2, std::enable_if_t<(std::is_signed_v<T1>&& std::is_unsigned_v<T2>) || (std::is_unsigned_v<T1> && std::is_signed_v<T2>)>> { using S = std::conditional_t<std::is_signed_v<T1>, T1, T2>; using U = std::conditional_t<std::is_unsigned_v<T1>, T1, T2>; static constexpr std::size_t larger_size = (sizeof(S) > sizeof(U)) ? sizeof(S) : sizeof(U); using type = std::conditional_t<larger_size <= 4, std::conditional_t<larger_size == 1 || (sizeof(S) == 2 && sizeof(U) == 1), std::int16_t, std::conditional_t<larger_size == 2 || (sizeof(S) == 4 && sizeof(U) < 4), std::int32_t, std::int64_t>>, std::conditional_t<sizeof(U) == 8, std::uint64_t, std::int64_t>>; };
	template <typename T1, typename T2> using common_index_type_t = typename common_index_type<T1, T2>::type;

	enum tp : opt_t {
		none = 0,
		priority = 1 << 0,
		pause = 1 << 2,
		wait_deadlock_checks = 1 << 3
	};

	using light_thread_pool = thread_pool<tp::none>;
	using priority_thread_pool = thread_pool<tp::priority>;
	using pause_thread_pool = thread_pool<tp::pause>;
	using wdc_thread_pool = thread_pool<tp::wait_deadlock_checks>;

	template <opt_t OptFlags = tp::none>
	class thread_pool {
	public:
		static constexpr bool priority_enabled = (OptFlags & tp::priority) != 0;
		static constexpr bool pause_enabled = (OptFlags & tp::pause) != 0;
		static constexpr bool wait_deadlock_checks_enabled = (OptFlags & tp::wait_deadlock_checks) != 0;
		thread_pool() : thread_pool(0, [] {}) {}

		explicit thread_pool(std::size_t n) : thread_pool(n, [] {}) {}

		template <typename F>
		explicit thread_pool(F&& init) : thread_pool(0, std::forward<F>(init)) {}

		template <typename F>
		thread_pool(std::size_t n, F&& init)
		{
			create_threads(n, std::forward<F>(init));
		}
		thread_pool(const thread_pool&) = delete;
		thread_pool(thread_pool&&) = delete;
		thread_pool& operator=(const thread_pool&) = delete;
		thread_pool& operator=(thread_pool&&) = delete;
		~thread_pool() noexcept { try { wait(); } catch (...) {} }

		//template <typename T1, typename T2, typename T = common_index_type_t<T1, T2>, typename F>
		//void detach_loop(const T1 first, const T2 last, F&& loop, std::size_t n = 0, priority_t p = 0) {
		//	if (static_cast<T>(last) > static_cast<T>(first)) {
		//		auto loop_ptr = std::make_shared<std::decay_t<F>>(std::forward<F>(loop));
		//		blocks blks(static_cast<T>(first), static_cast<T>(last), n ? n : thread_count);
		//		for (std::size_t blk = 0; blk < blks.get_num_blocks(); ++blk) {
		//			detach_task([loop_ptr, start = blks.start(blk), end = blks.end(blk)] { for (T i = start; i < end; ++i) (*loop_ptr)(i); }, p);
		//		}
		//	}
		//}

		//template <typename T1, typename T2, typename T = common_index_type_t<T1, T2>, typename F>
		//void detach_sequence(const T1 first, const T2 last, F&& seq, priority_t p = 0) {
		//	if (static_cast<T>(last) > static_cast<T>(first)) {
		//		auto seq_ptr = std::make_shared<std::decay_t<F>>(std::forward<F>(seq));
		//		for (T i = static_cast<T>(first); i < static_cast<T>(last); ++i) {
		//			detach_task([seq_ptr, i] { (*seq_ptr)(i); }, p);
		//		}
		//	}
		//}

		template <typename F>
		void detach_task(F&& task, priority_t p = 0) {
			{ std::scoped_lock l(tasks_mutex); if constexpr (priority_enabled) tasks.emplace(std::forward<F>(task), p); else tasks.emplace(std::forward<F>(task)); }
			task_available_cv.notify_one();
		}

		std::size_t get_tasks_queued() const { std::scoped_lock l(tasks_mutex); return tasks.size(); }
		std::size_t get_tasks_running() const { std::scoped_lock l(tasks_mutex); return tasks_running; }
		std::size_t get_tasks_total() const { std::scoped_lock l(tasks_mutex); return tasks_running + tasks.size(); }
		std::size_t get_thread_count() const noexcept { return thread_count; }
		std::vector<thread_t::id> get_thread_ids() const { std::vector<thread_t::id> ids(thread_count); for (std::size_t i = 0; i < thread_count; ++i) ids[i] = threads[i].get_id(); return ids; }

		template <typename F>
		void set_cleanup_func(F&& cleanup)
		{
			cleanup_func = [c = std::forward<F>(cleanup)](std::size_t i) { c(i); };
		}

		template <typename T1, typename T2, typename T = common_index_type_t<T1, T2>, typename F>
		multi_future<void> submit_loop(const T1 first, const T2 last, F&& loop, std::size_t n = 0, priority_t p = 0) {
			if (static_cast<T>(last) > static_cast<T>(first)) {
				auto loop_ptr = std::make_shared<std::decay_t<F>>(std::forward<F>(loop));
				blocks blks(static_cast<T>(first), static_cast<T>(last), n ? n : thread_count);
				multi_future<void> future; future.reserve(blks.get_num_blocks());
				for (std::size_t blk = 0; blk < blks.get_num_blocks(); ++blk) {
					future.push_back(submit_task([loop_ptr, start = blks.start(blk), end = blks.end(blk)] { for (T i = start; i < end; ++i) (*loop_ptr)(i); }, p));
				}
				return future;
			}
			return {};
		}

		template <typename T1, typename T2, typename T = common_index_type_t<T1, T2>, typename F, typename R = std::invoke_result_t<std::decay_t<F>, T>>
		multi_future<R> submit_sequence(const T1 first, const T2 last, F&& seq, priority_t p = 0) {
			if (static_cast<T>(last) > static_cast<T>(first)) {
				auto seq_ptr = std::make_shared<std::decay_t<F>>(std::forward<F>(seq));
				multi_future<R> future; future.reserve(static_cast<std::size_t>(static_cast<T>(last) > static_cast<T>(first)));
				for (T i = static_cast<T>(first); i < static_cast<T>(last); ++i) {
					future.push_back(submit_task([seq_ptr, i] { return (*seq_ptr)(i); }, p));
				}
				return future;
			}
			return {};
		}

		template <typename F, typename R = std::invoke_result_t<std::decay_t<F>>>
		std::future<R> submit_task(F&& task, priority_t p = 0) {
			auto promise = std::make_shared<std::promise<R>>();
			std::future<R> future = promise->get_future();
			detach_task([task = std::forward<F>(task), promise]() mutable {
				try { if constexpr (std::is_void_v<R>) { task(); promise->set_value(); } else { promise->set_value(task()); } }
				catch (...) { try { promise->set_exception(std::current_exception()); } catch (...) {} }
				}, p);
			return future;
		}

		void wait() {
			std::unique_lock tasks_lock(tasks_mutex);
			waiting = true;
			tasks_done_cv.wait(tasks_lock, [this] { if constexpr (pause_enabled) return (tasks_running == 0) && (paused || tasks.empty()); else return (tasks_running == 0) && tasks.empty(); });
			waiting = false;
		}

		template <typename R, typename P> bool wait_for(const std::chrono::duration<R, P>& duration) {
			std::unique_lock tasks_lock(tasks_mutex);
			waiting = true;
			const bool status = tasks_done_cv.wait_for(tasks_lock, duration, [this] { if constexpr (pause_enabled) return (tasks_running == 0) && (paused || tasks.empty()); else return (tasks_running == 0) && tasks.empty(); });
			waiting = false;
			return status;
		}

		template <typename C, typename D> bool wait_until(const std::chrono::time_point<C, D>& timeout_time) {
			std::unique_lock tasks_lock(tasks_mutex);
			waiting = true;
			const bool status = tasks_done_cv.wait_until(tasks_lock, timeout_time, [this] { if constexpr (pause_enabled) return (tasks_running == 0) && (paused || tasks.empty()); else return (tasks_running == 0) && tasks.empty(); });
			waiting = false;
			return status;
		}

		void purge() { std::scoped_lock l(tasks_mutex); tasks = {}; }
		void reset() { reset(0, [](std::size_t) {}); }
		void reset(std::size_t n) { reset(n, [](std::size_t) {}); }

		template <typename F> void reset(F&& init) {
			reset(0, std::forward<F>(init));
		}

		template <typename F> void reset(std::size_t n, F&& init) {
			if constexpr (pause_enabled) {
				std::unique_lock l(tasks_mutex); const bool was_paused = paused; paused = true; l.unlock(); reset_pool(n, std::forward<F>(init)); l.lock(); paused = was_paused;
			}
			else { reset_pool(n, std::forward<F>(init)); }
		}

		template <typename F> 
		void create_threads(std::size_t n, F&& init) {
			init_func = [init = std::forward<F>(init)](std::size_t i) { init(i); };
			thread_count = n > 0 ? n : (thread_t::hardware_concurrency() > 0 ? thread_t::hardware_concurrency() : 1);
			threads = std::make_unique<thread_t[]>(thread_count);
			{ std::scoped_lock l(tasks_mutex); tasks_running = thread_count; }
			for (std::size_t i = 0; i < thread_count; ++i) {
				threads[i] = thread_t([this, i](const std::stop_token& stop_token) { worker(stop_token, i); });
			}
		}

		void worker(const std::stop_token& stop_token, std::size_t idx) {
			init_func(idx);
			while (true) {
				std::unique_lock l(tasks_mutex);
				--tasks_running;
				if (waiting && (tasks_running == 0) && (pause_enabled ? (paused || tasks.empty()) : tasks.empty())) tasks_done_cv.notify_all();
				task_available_cv.wait(l, stop_token, [this] { return !(pause_enabled ? (paused || tasks.empty()) : tasks.empty()); });
				if (stop_token.stop_requested()) break;
				{ task_t task = pop_task(); ++tasks_running; l.unlock(); try { task(); } catch (...) {} }
			}
		}

		task_t pop_task() {
			task_t task;
			if constexpr (priority_enabled) task = std::move(const_cast<pr_task&>(tasks.top()).task);
			else task = std::move(tasks.front());
			tasks.pop();
			return task;
		}

	private:
		function_t<void(std::size_t)> cleanup_func = [](std::size_t) {};
		function_t<void(std::size_t)> init_func = [](std::size_t) {};
		std::conditional_t<pause_enabled, bool, std::monostate> paused = {};
		std::condition_variable_any task_available_cv;
		std::condition_variable tasks_done_cv;
		std::conditional_t < priority_enabled,
			std::priority_queue<pr_task>,
			std::queue<task_t >> tasks;
		mutable std::mutex tasks_mutex;
		std::size_t tasks_running = 0;
		std::size_t thread_count = 0;
		std::unique_ptr<thread_t[]> threads = nullptr;
		bool waiting = false;
	};

	class synced_stream {
	public:
		explicit synced_stream() { add_stream(std::cout); }
		template <typename... T> explicit synced_stream(T&... streams) { (add_stream(streams), ...); }
		void add_stream(std::ostream& stream) { out_streams.push_back(&stream); }
		std::vector<std::ostream*>& get_streams() noexcept { return out_streams; }
		template <typename... T> void print(const T&... items) { std::scoped_lock l(stream_mutex); for (auto* s : out_streams) (*s << ... << items); }
		template <typename... T> void println(T&&... items) { print(std::forward<T>(items)..., '\n'); }
		void remove_stream(std::ostream& stream) { out_streams.erase(std::remove(out_streams.begin(), out_streams.end(), &stream), out_streams.end()); }
		inline static std::ostream& (&endl)(std::ostream&) = static_cast<std::ostream & (&)(std::ostream&)>(std::endl);
		inline static std::ostream& (&flush)(std::ostream&) = static_cast<std::ostream & (&)(std::ostream&)>(std::flush);
	private:
		std::vector<std::ostream*> out_streams;
		mutable std::mutex stream_mutex;
	};
	using binary_semaphore = std::binary_semaphore;
	template <std::ptrdiff_t LeastMaxValue = std::counting_semaphore<>::max()>
	using counting_semaphore = std::counting_semaphore<LeastMaxValue>;
} // namespace plib::core::concurrent
#endif