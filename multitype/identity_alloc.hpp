//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

template <typename T>
struct identity_allocator {

	using data_t = T;

	/// The allocator type that provides the T allocate function
	///
	/// \tparam Alloc the allocator type that can allocate data_t obejcts
	/// All storage in this storage is allocator-wide accessible
	template <typename Alloc, bool check_oom, bool check_free_null>
	struct type {
		// identity_alloc does not provide any functions as everything is implemented by the
		// upper allocator

		// nonce free so that it can be found from upper allocators
		constexpr void free() {}
		// same with allocate
		constexpr void allocate() {}
	};
};
