//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

template<typename T, unsigned N>
class dynamic_storage {
	T data[N];

public:
	using data_t = T;
	using ptr_t = T*;

	static constexpr unsigned max_elems = N;

	constexpr unsigned index_of(ptr_t ptr) {
		return unsigned(ptr - data);
	}

	constexpr ptr_t at(unsigned pos) {
		return &data[pos];
	}
};
