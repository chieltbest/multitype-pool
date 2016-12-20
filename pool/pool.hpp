//
// Created by chiel on 26/10/16.
//

#ifndef MULTITYPE_POOL_POOL_HPP
#define MULTITYPE_POOL_POOL_HPP

#include <memory>
#include "../autosize/autosize_int.hpp"

void null_func() {}

/// general pool interface, handles initialization and deconstructing of elements, and out of memory errors
template<typename DataPolicy, typename DefaultAllocate = void, void (*OutOfMemoryPolicy)() = null_func>
class pool { // TODO give this a better name
	DataPolicy data;

public:
	template<typename T>
	using ptr_t = typename DataPolicy::template ptr_t<T>;

	constexpr pool()
			: data{} {
	}

	/// allocate and construct a single element with the arguments `args...`
	template<typename T = DefaultAllocate, typename ...Args>
	ptr_t<T> allocate(Args&&... args) {
		ptr_t<T> new_elem{data.template allocate<T>()};
		if (new_elem == nullptr) {
			OutOfMemoryPolicy();
		}

		// initialize and construct the element for the provided type
		new (&*new_elem) T(args...);
		return new_elem;
	}

	/// deconstruct and free a single element
	template<typename T>
	void free(ptr_t<T> ptr) {
		(*ptr).~T();
		data.free(ptr);
	}

};

#endif //MULTITYPE_POOL_POOL_HPP
