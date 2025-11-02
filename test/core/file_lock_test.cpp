#include "utils/file_lock.hpp"
#include <stdio.h>
#include <iostream>

int main(){
    FILE* file  = fopen("test/file_lock_testfile.txt", "w+");
    if(!file){
        std::cerr<<"Failed to open file for testing file lock."<<std::endl;
        return 1;
    }
    return 0;
}