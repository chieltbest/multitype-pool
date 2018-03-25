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

#include "multitype/combine/greedy.hpp"
#include "mpl/test/functional/bind.hpp"

// stored type 1
struct foo {
	int bar[4];
};

// stored type 2
struct bar {
	int qux;
};

template <unsigned N>
struct arr {
	int barr[N];
};

using identity_foo = segmented_allocator<kvasir::mpl::uint_<2>, identity_allocator<foo>>;
using identity_bar = segmented_allocator<kvasir::mpl::uint_<4>, identity_allocator<bar>>;
using union_foobar = union_alloc<identity_foo, identity_bar>;
using seg_8_foobar = segmented_allocator<kvasir::mpl::uint_<16>, union_foobar>;

//using new_pool_segment_t = segmented_allocator<
//        4, segmented_allocator<8, union_alloc<identity_allocator<foo>, identity_allocator<bar>>>>;
using bitmap_alloc_t = bitmap_alloc<1, seg_8_foobar, true, true>;

template <typename... Ts>
using multitype_pool = kvasir::mpl::call<
        greedy_combine<kvasir::mpl::cfe<segmented_allocator>, kvasir::mpl::cfe<union_alloc>>,
        identity_allocator<Ts>...>;

template <typename... AllocTypes>
void test_alloc() {
	using Alloc = bitmap_alloc<2, multitype_pool<AllocTypes...>, true, true>;
	Alloc alloc;

	std::cout << Alloc::usage << " " << sizeof(Alloc) << " "
	          << Alloc::usage / sizeof(Alloc) / sizeof...(AllocTypes) << std::endl;

	//	bitmap_alloc_t alloc{};

	std::cout << sizeof(alloc) << std::endl;

	foo *foo_ptrs[40]{nullptr};
	int foo_i = 0;
	bar *bar_ptrs[40]{nullptr};
	int bar_i = 0;

	for (unsigned i = 0; i < 40; ++i) {
		if ((i ^ (i >> 1u)) & 1u) {
			foo_ptrs[foo_i] = alloc.allocate(tag<foo>{}, foo{1, 2, 3, 4});
			std::cout << foo_ptrs[foo_i] << " foo" << std::endl;
			++foo_i;
		} else {
			bar_ptrs[bar_i] = alloc.allocate(tag<bar>{}, bar{1});
			std::cout << bar_ptrs[bar_i] << " bar" << std::endl;
			++bar_i;
		}
	}

	std::cout << std::endl;

	for (int i = 0; i < foo_i; ++i) {
		std::cout << foo_ptrs[i] << " foo" << std::endl;
		alloc.free(foo_ptrs[i]);
	}
	for (int i = 0; i < bar_i; ++i) {
		std::cout << bar_ptrs[i] << " bar" << std::endl;
		alloc.free(bar_ptrs[i]);
	}
}

int main() {
	test_alloc<foo, bar, arr<5>, arr<6>>();

	return 0;
}
