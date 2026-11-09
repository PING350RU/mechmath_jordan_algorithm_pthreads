cmake_minimum_required(VERSION 3.14)

project(JordanInverse LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(jordan main.cpp matrix.cpp jordan.cpp)
target_link_libraries(jordan PRIVATE Threads::Threads)
