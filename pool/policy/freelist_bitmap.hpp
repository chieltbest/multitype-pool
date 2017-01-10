//
// Created by chiel on 28/12/16.
//

#ifndef MULTITYPE_POOL_FREELIST_BITMAP_HPP
#define MULTITYPE_POOL_FREELIST_BITMAP_HPP

#include <stdint.h>
#include <cmath>
#include "../../autosize/autosize.hpp"

namespace {
#if __has_builtin(__builtin_ffsll)
	template<typename T>
	constexpr static int ffs(T num) {
		return __builtin_ffsll(num);
	}
#else
	static const int MultiplyDeBruijnBitPosition[32] = {
		0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
		31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9};

	/// find the least significant bit set in a number, returning 0 if the number is 0
	template<typename T>
	constexpr static int ffs(T num) {
		return MultiplyDeBruijnBitPosition[((uint32_t) ((num & -num) * 0x077CB531U)) >> 27];
	}
#endif
}

/// non-atomic bitmap, to be used when it is sure that there is
/// only ever one thread accessing the pool object
template<typename StoragePolicy>
class freelist_bitmap {
public:
	using data_t = typename StoragePolicy::data_t;
	using ptr_t = typename StoragePolicy::ptr_t;

private:
	StoragePolicy data;
	/// a bitmap where each bit represents a single element
	/// free elements are signified by a 1 bit, allocated ones by a 0
	autosize::uint_sizeof<(StoragePolicy::max_elems + 7) / 8> free; // round value up

public:
	constexpr freelist_bitmap()
		: data{},
		  free{(1 << StoragePolicy::max_elems) - 1} {
		// initialize the bitmap to have all elements nessecary free, but the others zero, as to
		// prevent bad allocations/counting
	}

	void push(ptr_t ptr) {
		free |= (1 << data.index_of(ptr)); // set the correct bit corresponding to the element
	}

	ptr_t pop() {
		auto pos = ffs(free);
		ptr_t ptr = data.at(pos);
		// TODO handle out of memory?
		free &= ~(1 << pos);
		return ptr;
	}

	auto get_free() {
#if __has_builtin(__builtin_popcount)
		return __builtin_popcount(free);
#else
		int count = 0, tmp = free;
		for (; tmp != 0; tmp &= tmp - 1)
			count++;
		return count;
#endif
	}

};

#endif //MULTITYPE_POOL_FREELIST_BITMAP_HPP
