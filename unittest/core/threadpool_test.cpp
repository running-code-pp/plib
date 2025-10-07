#include <cstdio>
#include <chrono>
#include <thread>
#include "concurrent/threadpool.hpp"
using namespace plib::core::concurrent;

int main(){
    ThreadPool<option_t::priority> pool([]() {
        // 由于 std::this_thread::get_id() 不容易直接转换为数字，这里仅输出提示信息
        printf("thread %d start\n",std::this_thread::get_id());
        fflush(stdout);
    });

    for (int i = 0; i < 100; i++) {
        pool.submit_task([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            printf("task func was finished in thread\n");
            fflush(stdout);
        });
    }

    auto return_func = []()->std::string {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return "hello world!";
        };
    auto test = pool.submit_task(return_func);
    printf("%s\n", test.get().c_str());
    std::this_thread::sleep_for(std::chrono::seconds(3));
    printf("task queue size:%d\n", pool.get_tasks_queued());
    printf("task running num:%d\n", pool.get_tasks_running());
    return 0;
}

