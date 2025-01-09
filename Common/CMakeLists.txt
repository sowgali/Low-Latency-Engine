cmake_minimum_required(VERSION 3.10)
project(LowLatencyTradingEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wpedantic")
set(CMAKE_VERBOSE_MAKEFILE on)

file(GLOB_RECURSE SOURCES "*.cpp")

include_directories(${PROJECT_SOURCE_DIR})

add_library(libcommon STATIC ${SOURCES})

list(APPEND LIBS libcommon)
list(APPEND LIBS pthread)

add_executable(thread_example examples/thread_example.cpp)
target_link_libraries(thread_example PUBLIC ${LIBS})

add_executable(mempool_example examples/mempool_example.cpp)
target_link_libraries(mempool_example PUBLIC ${LIBS})

add_executable(lf_example examples/spsc_lf_queue_example.cpp)
target_link_libraries(lf_example PUBLIC ${LIBS})

add_executable(logging_example examples/logging_example.cpp)
target_link_libraries(logging_example PUBLIC ${LIBS})