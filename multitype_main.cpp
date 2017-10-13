//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include "multitype/segmented_storage.hpp"
#include "multitype/union_alloc.hpp"

template<typename ...Ts>
union union_ : Ts... {};

// stored type 1
struct foo {
	int bar[4];
};

struct bar {
	int qux;
};

// lowest level allocator segment
using pool_seg = seg_alloc_segment<foo, 4>;
union foo_bar_seg {
	pool_seg a;
	bar b;
};
// upper level segment allocator segment
using pool_seg_seg = seg_alloc_segment<pool_seg, 4>;

// top level allocator
using pool_seg_seg_alloc = bitmap_alloc<pool_seg_seg, 8>;
// upper level segment allocator
using pool_seg_alloc = segmented_allocator<pool_seg_seg_alloc>;
// the actual allocator
using pool_t = segmented_allocator<pool_seg_alloc>;


int main() {
	std::cout << sizeof(foo) << " " << sizeof(pool_seg) << " " << sizeof(pool_seg_alloc) << " "
	          << sizeof(pool_t) << std::endl;

	pool_t pool{};

	foo *ptrs[20]{nullptr};

	for (foo *&ptr : ptrs) {
		ptr = pool.allocate(foo{1, 2, 3, 4});

		std::cout << ptr << std::endl;
	}

	std::cout << std::endl;

	for (foo *ptr : ptrs) {
		std::cout << ptr << std::endl;
		pool.free(ptr);
	}

	return 0;
}
