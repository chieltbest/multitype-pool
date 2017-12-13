//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
//#pragma pack(push, 1)
#include "multitype/segmented_storage.hpp"
#include "multitype/union_alloc.hpp"

#include "mpl/src/kvasir/mpl/types/bool.hpp"
//#pragma pack(pop)

// stored type 1
struct foo {
	int bar[4];
};

struct bar {
	int qux;
};
/*
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
using pool_t = segmented_allocator<pool_seg_alloc>;*/

using new_pool_segment_t = segmented_allocator<
        4, segmented_allocator<8, union_alloc<identity_allocator<foo>, identity_allocator<bar>>>>;
using bitmap_alloc_t = bitmap_alloc<1, new_pool_segment_t, true, true>;


int main() {
	bitmap_alloc_t alloc{};

	std::cout << sizeof(typename new_pool_segment_t::data_t) << " " << sizeof(alloc) << std::endl;

	foo *ptrs[20]{nullptr};

	for (foo *&ptr : ptrs) {
		ptr = alloc.allocate(tag<foo>{}, foo{1, 2, 3, 4});

		std::cout << ptr << std::endl;
	}

	std::cout << std::endl;

	for (foo *ptr : ptrs) {
		std::cout << ptr << std::endl;
		alloc.free(ptr);
	}

	return 0;
}
