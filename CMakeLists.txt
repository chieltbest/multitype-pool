cmake_minimum_required(VERSION 3.1)
project(multitype_pool)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpadded -fno-exceptions -fsanitize=undefined \
-ftemplate-backtrace-limit=0")
# -fsanitize=undefined

add_subdirectory(mpl)

add_executable(multitype_main multitype_main.cpp
		autosize/autosize.hpp
		multitype/allocator.hpp
		multitype/identity_alloc.hpp
		multitype/segmented_storage.hpp
		multitype/union_alloc.hpp
		multitype/combine/combine.hpp
		multitype/combine/greedy.hpp
		multitype/root/bitmap_alloc.hpp
		multitype/root/single_alloc.hpp)
target_link_libraries(multitype_main kvasir_mpl)
