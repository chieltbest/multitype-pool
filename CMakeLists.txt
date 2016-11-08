cmake_minimum_required(VERSION 3.6)
project(multitype_pool)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpadded")

set(SOURCE_FILES main.cpp)
add_executable(multitype_pool ${SOURCE_FILES})