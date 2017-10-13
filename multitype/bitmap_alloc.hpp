//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include "../pool/policy/freelist_bitmap.hpp"
#include "../pool/policy/dynamic_storage.hpp"

/// bitmap allocator compatible with the segmented storage allocator
template<typename T, std::size_t N>
class bitmap_alloc {
	T data[N];
	/// a bitmap where each bit represents a single element
	/// free elements are signified by a 1 bit, allocated ones by a 0
	autosize::uint_sizeof<(N + 7) / 8> free_bits = (1 << N) - 1; // round value up

public:
	using data_t = T;
	using ptr_t = T*;

	T& lookup_type(const char* ptr) {
		std::ptrdiff_t diff = ptr - ((const char*) &data[0]);
		// truncate to the index
		std::ptrdiff_t idx = diff / sizeof(T);
		return data[idx];
	}

	void free(const T *ptr) {
		free_bits |= (1 << (ptr - &data[0])); // set the correct bit corresponding to the element
	}

	template<typename ...Args>
	ptr_t allocate(Args&&...args) {
		auto pos = get_free_pos(free_bits);

		ptr_t ptr = new (&data[pos]) T(args...);

		// clear the free bit
		free_bits ^= 1 << pos;
		return ptr;
	}

	bool full() {
		return free_bits == 0;
	}

	bool empty() {
		return free_bits == (1 << N) - 1;
	}

private:
	/// find the least significant bit set in a number, being unspecified if the number is 0
	template<typename Num>
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
