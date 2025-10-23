# Tests directory CMakeLists.txt

# Enable testing
enable_testing()

# Find all test source files recursively
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# Only create test executable if source files exist
if(TEST_SOURCES)
    # Test executable
    add_executable(plib_tests core/core_test.cpp)
    # Link with plib and GTest
    find_package(GTest REQUIRED)
    target_link_libraries(plib_tests
        plib-core
        yaml-cpp::yaml-cpp
        GTest::gtest
        GTest::gtest_main
    )
    
    # Add test
        add_test(NAME plib_tests COMMAND plib_tests --gtest_catch_exceptions=0)
else()
    message(WARNING "No test source files found in test directory")
endif() 


# 添加独立的线程池测试可执行文件，不使用 gtest
add_executable(threadpool_test core/threadpool_test.cpp)
target_link_libraries(threadpool_test plib-core)

# Optional: Google Benchmark based benchmarks
find_package(benchmark QUIET)
if(benchmark_FOUND)
    file(GLOB_RECURSE BENCH_SOURCES "*.bench.cpp" "bench_*.cpp")
    if(BENCH_SOURCES)
        add_executable(plib_benchmarks ${BENCH_SOURCES})
        target_link_libraries(plib_benchmarks
            plib-core
            benchmark::benchmark
        )
        # Don't run benchmarks as CTest by default, but provide a target
        add_custom_target(run_benchmarks COMMAND plib_benchmarks DEPENDS plib_benchmarks)
    else()
        message(STATUS "No benchmark sources found (searching *.bench.cpp or bench_*.cpp)")
    endif()
else()
    message(STATUS "Google Benchmark not found - skipping benchmark targets")
endif()