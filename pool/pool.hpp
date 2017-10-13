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
	template<typename T = DefaultAllocate>
	using value_type = T;
	template <typename T = DefaultAllocate>
	using ptr_t          = typename DataPolicy::template ptr_t<T>;

	constexpr pool()
	    : data{} {
	}

	/// allocate a single element, bulk allocation is not supported at this moment
	template <typename T = DefaultAllocate>
	ptr_t<T> allocate() {
		ptr_t<T> new_elem{data.template allocate<T, OutOfMemoryPolicy != do_undefined_behaviour>()};
		if (new_elem == nullptr) {
			OutOfMemoryPolicy();
		}

		return new_elem;
	}

	/// call the constructor of a certain element
	template <typename T, typename... Args>
	void construct(ptr_t<T> ptr, Args &&... args) {
		// initialize and construct the element for the provided type
		// get the raw pointer type by using &*
		new (&*ptr) T(std::forward<Args>(args)...);
	};

	/// convenience function for simultaneously allocating and constructing an element
	template<typename T = DefaultAllocate, typename ...Args>
	ptr_t<T> create(Args&& ...args) {
		auto new_elem{allocate<T>()};
		construct(new_elem, std::forward<Args>(args)...);
		return new_elem;
	}


	/// destruct a single element
	template<typename T>
	void destroy(ptr_t<T> ptr) {
		(*ptr).~T();
	}

	/// deallocate a single element
	template <typename T>
	void deallocate(ptr_t<T> ptr) {
		data.free(ptr);
	}

	/// convenience function for simultaneously deconstructing and deallocating an element
	template<typename T>
	void free(ptr_t<T> ptr) {
		destroy(ptr);
		deallocate(ptr);
	}

	/// debugging function, do not use in production code
	auto get_free() {
		return data.get_free();
	}
};

template <typename Pool>
class stl_pool {
	constexpr static Pool data{};

public:
	/// stl pool interface
	template <typename T>
	class pool {
	public:
		using value_type = T;
		using pointer = typename Pool::template ptr_t<T>;

		// constructors are automatically generated

		pointer allocate(std::size_t num) {
			return data.template allocate<T>();
		}

		template <typename ...Args>
		void construct(Args&& ...args) {
			data.construct(std::forward<Args>(args)...);
		}

		void deallocate(pointer ptr) {
			data.deallocate(ptr);
		}

		void destroy(pointer ptr) {
			(*ptr).~T();
			data.free(ptr);
		}

		/// the maximum number of elements that this allocator can bulk allocate
		constexpr auto max_size() {
			return 1;
		}

	};
};

#endif //MULTITYPE_POOL_POOL_HPP
