//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <forward_list>

template<typename T>
class new_alloc {
	std::forward_list<T> storage;

public:
	T& lookup_type() {

	}

	template<typename ...Args>
	T* allocate(Args&& ...args) {
		storage.emplace_back(args...);
		return storage.end();
	}

	void free(const T* ptr) {
		storage.erase(ptr);
	}

};
