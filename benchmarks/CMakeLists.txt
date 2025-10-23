# benchmarks directory CMakeLists.txt

cmake_minimum_required(VERSION 3.15)

project(plib_benchmarks NONE)

include(CTest)
enable_testing()

# Try to find Google Benchmark (support both modern target name and older Gbenchmark)
find_package(benchmark QUIET)
if(NOT benchmark_FOUND)
    find_package(Gbenchmark QUIET)
endif()

if(benchmark_FOUND)
    set(BENCH_TARGET benchmark::benchmark)
    set(BENCH_MAIN_TARGET benchmark::benchmark_main)
    message(STATUS "Using target benchmark::benchmark")
elseif(TARGET Gbenchmark::gbenchmark OR Gbenchmark_FOUND)
    # Some packaging/use an older namespace
    set(BENCH_TARGET Gbenchmark::gbenchmark)
    set(BENCH_MAIN_TARGET Gbenchmark::gbenchmark_main)
    message(STATUS "Using target Gbenchmark::gbenchmark")
else()
    set(BENCH_TARGET "")
    set(BENCH_MAIN_TARGET "")
    message(STATUS "Google Benchmark not found - benchmark targets will be skipped")
endif()

# Collect benchmark sources in this directory (and core subdir)
file(GLOB BENCH_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     "*.cpp" "*.cc" "core/*.cpp" "core/*.cc")

if(NOT BENCH_SOURCES)
    message(WARNING "No benchmark source files found in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

set(STANDALONE_BENCHES "threadpool_benchmark.cpp")

# Create standalone (non-Google-Benchmark) executables first
foreach(b IN LISTS STANDALONE_BENCHES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/${b}")
        get_filename_component(name ${b} NAME_WE)
        add_executable(${name} core/${b})
        target_link_libraries(${name} PRIVATE plib-core)
        add_test(NAME ${name} COMMAND ${name})
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${b}")
        get_filename_component(name ${b} NAME_WE)
        add_executable(${name} ${b})
        target_link_libraries(${name} PRIVATE plib-core)
        add_test(NAME ${name} COMMAND ${name})
    endif()
endforeach()

# If benchmark library is available, create an executable per benchmark source
if(BENCH_TARGET)
    foreach(src IN LISTS BENCH_SOURCES)
        # skip standalone benches and helper files
        if(src MATCHES "threadpool_benchmark\\.cpp$")
            continue()
        endif()

        get_filename_component(bname ${src} NAME_WE)
        string(REPLACE "." _safe_name ${bname} target_suffix)
        set(target_name plib_bench_${bname})

        add_executable(${target_name} ${src})
        target_link_libraries(${target_name} PRIVATE plib-core ${BENCH_TARGET} ${BENCH_MAIN_TARGET})
        target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/core)

        # register with CTest so `ctest` can run benchmarks (they will execute fast; you can pass args)
        add_test(NAME ${target_name} COMMAND ${target_name})
    endforeach()
else()
    message(STATUS "Skipping generation of Google Benchmark targets because benchmark library is missing.")
endif()
