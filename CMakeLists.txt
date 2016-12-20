cmake_minimum_required(VERSION 3.1)
project(multitype_pool)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpadded -fno-exceptions")
# -fsanitize=undefined

set(SOURCE_FILES main.cpp)
add_executable(multitype_pool ${SOURCE_FILES})
target_link_libraries(multitype_pool pthread atomic)