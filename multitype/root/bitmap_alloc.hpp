//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include "../../pool/policy/freelist_bitmap.hpp"
#include "../../pool/policy/dynamic_storage.hpp"

/// bitmap allocator compatible with the segmented storage allocator
///
/// \tparam N the number of elements to keep reserved
/// \tparam T the lower allocator to provide allocations to
/// \tparam CheckOom check for out of memory situations
/// \tparam CheckFreeNull check for calling free on null pointers
template <unsigned N, typename T, bool check_oom = false, bool check_free_null = false>
class bitmap_alloc : public T::template type<bitmap_alloc<N, T, check_oom, check_free_null>,
                                             check_oom, check_free_null> {
	using base_t = typename T::template type<bitmap_alloc<N, T, check_oom, check_free_null>,
	                                         check_oom, check_free_null>;
	using data_t = typename T::data_t;

	// data has to be left undefined because it could happen that the default constructor is
	// undefined
	data_t data[N];

	/// a bitmap where each bit represents a single element
	/// free elements are signified by a 1 bit, allocated ones by a 0
	using free_bits_t = autosize::uint_sizeof<(N + 7) / 8>; // round value up
	constexpr static free_bits_t empty_bitset = (~0ull) >>
	                                            (sizeof(free_bits_t) * 8 - N);
	free_bits_t free_bits = empty_bitset;

public:
	constexpr static float usage = T::usage * N;

	constexpr bitmap_alloc()
	    : base_t{} {
	}

	void free(data_t *ptr) {
		if (!check_free_null || ptr) {
			free_bits |=
			        (1 << (ptr - &data[0])); // set the correct bit corresponding to the element
		}
	}
	using base_t::free;

	template <typename... Args>
	data_t *allocate(tag<data_t>, Args &&... args) {
		std::cout << std::string(detail::type_name<data_t>{}) << " balloc =" << std::endl;
		if (check_oom && full()) {
			return nullptr;
		}

		auto pos = get_free_pos(free_bits);

		auto *ptr = new (&data[pos]) data_t(std::forward<Args>(args)...);

		// clear the free bit
		free_bits ^= 1 << pos;
		return ptr;
	}
	using base_t::allocate;

	data_t &lookup_type(const char *ptr) {
		std::ptrdiff_t diff = ptr - ((const char *)&data[0]);
		// truncate to the index
		std::ptrdiff_t idx = diff / sizeof(data_t);
		return data[idx];
	}

	bool full() {
		return free_bits == 0;
	}

	bool empty() {
		return free_bits == empty_bitset;
	}

private:
	/// find the least significant bit set in a number, being unspecified if the number is 0
	template <typename Num>
	constexpr static int get_free_pos(Num num) {
#if __GNUC__
		return __builtin_ctzll(num);
#else
// use the biggest type available for the ctz instruction
#if __has_builtin(__builtin_ctzll)
		return __builtin_ctzll(num);
#else
#if __has_builtin(__builtin_ctzl)
		return __builtin_ctzl(num);
#else
#if __has_builtin(__builtin_ctz)
		return __builtin_ctz(num);
#else
		constexpr int MultiplyDeBruijnBitPosition[] = {0,  1,  28, 2,  29, 14, 24, 3,  30, 22, 20,
		                                               15, 25, 17, 4,  8,  31, 27, 13, 23, 21, 19,
		                                               16, 7,  26, 12, 18, 6,  11, 5,  10, 9};

		return MultiplyDeBruijnBitPosition[((uint32_t)((num & -num) * 0x077CB531U)) >> 27];
#endif
#endif
#endif
#endif
	}

	auto get_free() {
#if __GNUC__
		return __builtin_popcount(free_bits);
#else
#if __has_builtin(__builtin_popcount)
		return __builtin_popcount(free);
#else
		int count = 0, tmp = free;
		for (; tmp != 0; tmp &= tmp - 1)
			count++;
		return count;
#endif
#endif
	}
};
