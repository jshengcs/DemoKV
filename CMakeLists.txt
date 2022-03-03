cmake_minimum_required(VERSION 3.0.0)
project(DemoKV VERSION 0.1.0)

include(CTest)
enable_testing()

aux_source_directory(${PROJECT_SOURCE_DIR}/bench DIR_BENCH_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRC_SRCS)

add_subdirectory(src)
add_subdirectory(bench)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
