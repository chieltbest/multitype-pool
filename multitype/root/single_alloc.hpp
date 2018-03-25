//          Copyright Chiel Douwes 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

template <typename T, bool check_oom = false, bool check_free_null = false>
class single_alloc : public T::template type<single_alloc<T, check_oom, check_free_null>, check_oom,
                                      check_free_null> {
	using base_t = typename T::template type<single_alloc<T, check_oom, check_free_null>, check_oom,
	                                         check_free_null>;
	using data_t = typename T::data_t;
	data_t data;
	bool is_full{false};

public:
	constexpr static float usage = T::usage;

	void free(data_t *ptr) {
		data.~data_t();
		is_full = false;
	}
	using base_t::free;

	template <typename... Args>
	data_t *allocate(tag<data_t>, Args &&... args) {
		if (check_oom && full()) {
			return nullptr;
		}
		is_full = true;
		return new (&data) data_t(std::forward<Args>(args)...);
	}
	using base_t::allocate;

	data_t &lookup_type(const char *ptr) {
		return data;
	}

	// dummy full and empty functions
	bool full() {
		return is_full;
	}

	bool empty() {
		return !is_full;
	}
};
