//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

template <template <typename SAlloc> class... SubAlloc>
struct union_alloc {
	template <typename SuperAlloc>
	struct type : SubAlloc<SuperAlloc>... {
		constexpr type(SuperAlloc &alloc)
		    : SubAlloc{alloc}... {
		}
	};
};
