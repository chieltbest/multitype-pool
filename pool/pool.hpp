//
// Created by chiel on 26/10/16.
//

#ifndef MULTITYPE_POOL_POOL_HPP
#define MULTITYPE_POOL_POOL_HPP

#include <memory>

#include "../autosize/autosize.hpp"

/// tag function that causes undefined behaviour to happen in an out of memory situation
/// this is the default for all allocators because of performance reasons
inline void do_undefined_behaviour() {
}
/// tag function to be used when the function should return nullptr
/// in an out of memory situation
inline void return_null() {
}

/// used if the stored data type does not depend on the type of the pointer
template <typename T>
struct discard_ptr_t { // TODO make a separate header
	template <typename ptr_t>
	using node = T;
};

/// general pool interface, handles initialization and deconstructing of elements, and out of memory errors
template <typename DataPolicy, void (*OutOfMemoryPolicy)() = do_undefined_behaviour,
          typename DefaultAllocate = typename DataPolicy::default_data_t>
class pool { // TODO: give this a better name
	DataPolicy data;

public:
	template <typename T = DefaultAllocate>
	using ptr_t          = typename DataPolicy::template ptr_t<T>;

	constexpr pool()
	    : data{} {
	}

	/// allocate and construct a single element with the arguments `args...`
	template <typename T = DefaultAllocate, typename... Args>
	ptr_t<T> allocate(Args &&... args) {
		ptr_t<T> new_elem{data.template allocate<T, OutOfMemoryPolicy != do_undefined_behaviour>()};
		if (new_elem == nullptr) {
			OutOfMemoryPolicy();
		}

		// initialize and construct the element for the provided type
		new (&*new_elem) T(args...);
		return new_elem;
	}

	/// deconstruct and free a single element
	template <typename T>
	void free(ptr_t<T> ptr) {
		(*ptr).~T();
		data.free(ptr);
	}

	/// debugging function, do not use in production code
	auto get_free() {
		return data.get_free();
	}
};

#endif //MULTITYPE_POOL_POOL_HPP
