#include "crashreport/crash_report.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <functional>

int main()
{
    std::cout << "=== Crash Report Test ===" << std::endl;
    
    std::string options = R"(
Select crash type:
   1. Divide by zero error
   2. Null pointer dereference
   3. Access violation
   4. Stack overflow (infinite recursion)
Enter option: )";

    std::cout << options;
    int option;
    std::cin >> option;
    
    // Initialize crash reporter
    auto crashReport = plib::core::crashreport::CrashReportWriter::GetInstance();
    crashReport->setReportPath("crash_reports/");
    crashReport->start();
    
    std::cout << "\nCrash reporter initialized. Triggering crash in 2 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    switch (option)
    {
    case 1:
    {
        std::cout << "Triggering divide by zero..." << std::endl;
        volatile int a = 1;
        volatile int b = 0;
        volatile int c = a / b; // 除零（可能不会崩溃，取决于编译器）
        std::cout << "Result: " << c << std::endl;
    }
    break;
    
    case 2:
    {
        std::cout << "Triggering null pointer dereference..." << std::endl;
        int *p = nullptr;
        *p = 42; // 空指针解引用 - 会触发 EXCEPTION_ACCESS_VIOLATION
        std::cout << *p << std::endl;
    }
    break;
    
    case 3:
    {
        std::cout << "Triggering access violation..." << std::endl;
        int *p = reinterpret_cast<int *>(0x12345678);
        *p = 99; // 访问无效内存地址
        std::cout << *p << std::endl;
    }
    break;
    
    case 4:
    {
        std::cout << "Triggering stack overflow..." << std::endl;
        std::function<void()> infinite_recursion = [&]() {
            char buffer[10000]; // 快速消耗栈空间
            buffer[0] = 0;
            infinite_recursion();
        };
        infinite_recursion();
    }
    break;
    
    default:
        std::cout << "Invalid option selected." << std::endl;
        return 1;
    }

    std::cout << "If you see this, the crash was not triggered!" << std::endl;
    return 0;
}