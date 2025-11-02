/** 
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-11-01 21:43:52
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-11-01 21:49:35
 * @FilePath: \plib\test\core\os_util_test.cpp
 * @Description: 测试系统工具函数
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
#include "utils/os_util.hpp"
using namespace plib::core;
#include <iostream>

// PASS
void test_kill_process_by_pid(p_pid_t pid){
    if(utils::KillProcessByPid(pid)){
        std::cout<<"Successfully killed process with PID: "<<pid<<std::endl;
    }else{
        std::cout<<"Failed to kill process with PID: "<<pid<<std::endl;
    }
}


void test_close_processes(const char*filename){
    if(utils::CloseProcesses(filename)){
        std::cout<<"Successfully closed processes using file: "<<filename<<std::endl;
    }else{
        std::cout<<"Failed to close processes using file: "<<filename<<std::endl;
    }
}

// 29384  C:\Users\33209\Desktop\install.txt
// 
// os_util_test pid filename
int main(int argc,char**argv){
    if(argc<3){
        std::cerr<<"Usage: os_util_test <pid> <filename>"<<std::endl;
        return 1;
    }
    p_pid_t pid = static_cast<p_pid_t>(atoi(argv[1]));
    const char* filename = argv[2];
    // test_kill_process_by_pid(pid);
    test_close_processes(filename);
    return 0;
}