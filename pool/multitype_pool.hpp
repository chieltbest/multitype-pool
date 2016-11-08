//
// Created by chiel on 28/10/16.
//

#ifndef MULTITYPE_POOL_MULTITYPE_POOL_HPP
#define MULTITYPE_POOL_MULTITYPE_POOL_HPP

template<unsigned long long max_elems, typename empty_func>
struct allocation_counter {
	std::atomic<auto_uint<max_elems>> elems;

	constexpr enable_counter()
		: elems{} {
		static_assert(!elems.is_lock_free(), "the memory pool allocation counter should have an atomic type");
	};

	void operator++() {
		auto_uint<max_elems> read{elems.load()};
		while (elems.compare_exchange_weak(read, read + 1)) {
			read = elems.load();
		}
	}
	void operator--() {
		auto_uint<max_elems> read{elems.load()};
		while (elems.compare_exchange_weak(read, read - 1)) {
			read = elems.load();
		}

		if (read - 1 == 0) {
			empty_func();
		}
	}
};
template<unsigned long long max_elems>
struct allocation_counter<max_elems, nullptr> {
	constexpr void operator++() {}
	constexpr void operator--() {}
};

#endif //MULTITYPE_POOL_MULTITYPE_POOL_HPP
