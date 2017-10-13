//
// Created by chiel on 18/01/17.
//

#include <iostream>
#include <forward_list>

#include "pool/policy/static_storage.hpp"
#include "pool/policy/freelist_stack.hpp"
#include "pool/policy/freelist_data_store.hpp"
#include "pool/pool.hpp"


// the stored type
template<typename ptr_t>
class foo {
	int bar;

public:
	foo(int bar)
		: bar{bar} {
	}

	template<typename Stream>
	friend auto&& operator<<(Stream &lhs, const foo &rhs) {
		return lhs << rhs.bar;
	}

};

// the type of the static data storage
using freelist_t = static_storage_array_data<
	freelist_stack_node<foo>::node,
	10>; // size of the pool
freelist_t data_storage{};

// type of the pool object, wrapping the data storage
using pool_t = pool<
	freelist_data_store<
		freelist_stack<
			static_storage<
				freelist_t,
				data_storage>>>>;
pool_t pool_obj{};

int main() {
	// using the provided convenience function
	auto ptr = pool_obj.create(10);
	std::cout << ptr << " " << *ptr << std::endl;
	pool_obj.free(ptr);

	// wrapping the pool in an stl allocator interface
	std::forward_list</*stored type without pointer template*/,
	                  stl_pool<pool_t>::pool> vec{};


	return 0;
}